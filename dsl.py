from __future__ import annotations
from dataclasses import dataclass, field
from typing import List, Dict, Tuple, Optional, Any
import re, ast
from openai import OpenAI
from datetime import datetime

@dataclass
class TeeIRCall:
    name: str
    args: List[str]
    output: Optional[str] = None
    meta: Dict[str, Any] = field(default_factory=dict)

    def __repr__(self) -> str:
        out = f" -> {self.output}" if self.output else ""
        return f"{self.name}({', '.join(self.args)}){out}"

_PLACEHOLDER = re.compile(r"\$[a-zA-Z_][a-zA-Z0-9_]*")

@dataclass
class DslArgument:
    raw: str

    @property
    def is_var(self) -> bool:
        return bool(_PLACEHOLDER.fullmatch(self.raw))

    @property
    def var_name(self) -> Optional[str]:
        return self.raw if self.is_var else None

@dataclass
class DslCall:
    name: str
    args: List[DslArgument]
    output: Optional[DslArgument]

    @classmethod
    def parse(cls, text: str) -> "DslCall":
        text = text.strip()
        if "->" not in text:
            raise ValueError(f"Bad assignment (missing '->'): {text}")
        call, out = text.split("->", 1)
        call = call.strip()
        out = out.strip()
        if out == "_":
            out_arg = None
        else:
            out_arg = DslArgument(out)

        m = re.match(r"([A-Za-z_][A-Za-z0-9_]*)\((.*)\)$", call)
        if not m:
            raise ValueError(f"Bad call: {call}")
        name = m.group(1)
        arg_str = m.group(2).strip()
        args: List[DslArgument] = []
        if arg_str:
            parts = [s.strip() for s in split_commas(arg_str)]
            args = [DslArgument(p) for p in parts]
        return cls(name=name, args=args, output=out_arg)


def split_commas(s: str) -> List[str]:
    out, buf, depth = [], [], 0
    for ch in s:
        if ch == '(':
            depth += 1
            buf.append(ch)
        elif ch == ')':
            depth -= 1
            buf.append(ch)
        elif ch == ',' and depth == 0:
            out.append(''.join(buf).strip())
            buf = []
        else:
            buf.append(ch)
    if buf:
        out.append(''.join(buf).strip())
    return out


@dataclass
class DslAssignment:
    fixed_line: bool
    content: str
    call: Optional[DslCall]

    @classmethod
    def parse(cls, text: str) -> "DslAssignment":
        text = text.strip()
        if not text:
            raise ValueError("Empty assignment")
        if text.startswith('#'):
            return cls(fixed_line=True, content=text[1:].strip(), call=None)
        return cls(fixed_line=False, content=text, call=DslCall.parse(text))


@dataclass
class DslRule:
    src: List[DslAssignment]
    dst: List[DslAssignment]
    raw: str

    @classmethod
    def parse(cls, rule_str: str) -> "DslRule":
        if '=>' not in rule_str:
            raise ValueError("Rule must contain '=>' delimiter")
        src_str, dst_str = rule_str.split('=>', 1)
        src_items = [DslAssignment.parse(x) for x in src_str.split(';') if x.strip()]
        dst_items = [DslAssignment.parse(x) for x in dst_str.split(';') if x.strip()]
        for it in src_items:
            if it.fixed_line:
                raise ValueError("SRC side cannot contain fixed lines (#...)")
        return cls(src=src_items, dst=dst_items, raw=rule_str)


@dataclass
class MatchResult:
    start: int
    end: int  # [start, end) on IR list
    bindings: Dict[str, str]  # $var -> concrete string


def try_bind(arg_pat: DslArgument, arg_val: str, bindings: Dict[str, str]) -> bool:
    if arg_pat.is_var:
        v = arg_pat.var_name
        assert v is not None
        if v in bindings:
            return bindings[v] == arg_val
        bindings[v] = arg_val
        return True
    else:
        return arg_pat.raw == arg_val


def match_one_call(pat: DslCall, ir: TeeIRCall, bindings: Dict[str, str]) -> bool:
    if pat.name != ir.name:
        return False
    if len(pat.args) != len(ir.args):
        return False
    if pat.output is not None:
        if ir.output is None:
            return False
        if not try_bind(pat.output, ir.output, bindings):
            return False
    for a_pat, a_val in zip(pat.args, ir.args):
        if not try_bind(a_pat, a_val, bindings):
            return False
    return True


def subseq_match(rule: DslRule, ir_list: List[TeeIRCall]) -> List[MatchResult]:
    """在 ir_list 中寻找 rule.src 的子序列匹配，返回所有命中位置与绑定。"""
    src_calls = [a.call for a in rule.src]
    results: List[MatchResult] = []
    n = len(ir_list)
    m = len(src_calls)
    if any(c is None for c in src_calls):
        raise ValueError("SRC calls missing")
    for i in range(0, n - m + 1):
        bindings: Dict[str, str] = {}
        ok = True
        for j, pat in enumerate(src_calls):
            assert pat is not None
            if not match_one_call(pat, ir_list[i + j], bindings):
                ok = False
                break
        if ok:
            results.append(MatchResult(start=i, end=i + m, bindings=bindings))
    return results


def instantiate_arg(arg: DslArgument, b: Dict[str, str]) -> str:
    if arg.is_var:
        v = arg.var_name
        assert v is not None
        return b.get(v, v)
    return arg.raw


def instantiate_call(call: DslCall, b: Dict[str, str]) -> TeeIRCall:
    args = [instantiate_arg(a, b) for a in call.args]
    output = instantiate_arg(call.output, b) if call.output is not None else None
    return TeeIRCall(name=call.name, args=args, output=output)


def rewrite(rule: DslRule, ir_list: List[TeeIRCall]) -> Tuple[List[TeeIRCall], List[MatchResult]]:
    matches = subseq_match(rule, ir_list)
    if not matches:
        return ir_list[:], []
    chosen: List[MatchResult] = []
    last_end = -1
    for m in matches:
        if m.start >= last_end:
            chosen.append(m)
            last_end = m.end
    out: List[TeeIRCall] = []
    cursor = 0
    for m in chosen:
        out.extend(ir_list[cursor:m.start])
        for dst in rule.dst:
            if dst.fixed_line:
                code = substitute_fixed_line(dst.content, m.bindings)
                out.append(TeeIRCall(name="RAW", args=[code]))
            else:
                out.append(instantiate_call(dst.call, m.bindings))
        cursor = m.end
    out.extend(ir_list[cursor:])
    return out, chosen


def substitute_fixed_line(line: str, b: Dict[str, str]) -> str:
    def repl(m: re.Match) -> str:
        v = m.group(0)
        return b.get(v, v)
    return _PLACEHOLDER.sub(repl, line)


class CEmitter:
    @staticmethod
    def emit(ir: TeeIRCall) -> str:
        n = ir.name
        a = ir.args
        o = ir.output
        if n == "COPY":
            return f"TEE_MemMove({a[0]}, {a[1]}, {a[2]});"
        if n == "MALLOC":
            assert o is not None, "MALLOC needs an output variable"
            return f"char* {o} = (char*)TEE_Malloc({a[0]});"
        if n == "ENC":
            return f"enc({a[0]}, {a[1]}, {a[2]});"
        if n == "READ":
            assert o is not None, "READ needs output"
            return f"char {o}[256] = {0};\nread({o});"
        if n == "WRITE":
            return f"write({a[0]});"
        if n == "HASH":
            return f"char {a[0]}[256] = {0};\nhash({a[0]}, {a[1]}, {a[2]});"
        if n == "IF":
            cond = a[0] + " " + a[1] + " " + a[2]
            return (
                "if (" + cond + ") {\n"
                "    return TEE_ERROR_BAD_PARAMETERS;\n"
                "}"
            )
        if n == "SNPRINT":
            fmt = a[2]
            sargs = ""
            for i in range(a[3]):
                sargs += "$chiper" + str(i) + ", "
            sargs = sargs.rstrip(", ")
            return f"snprintf({a[0]}, {a[1]}, {fmt}" + (", " + sargs if sargs else "") + ");"
        if n == "MULMALLOC":
            out = ""
            j = 0
            for i in a[0]:
                out += f"char* $chiper{j} = (char*)TEE_Malloc(strlen({i}));\n"
                j += 1
            out = out.rstrip("\n")
            return out
        if n == "MULENC":
            out = ""
            j = 0
            for i in a[0]:
                out += f"enc({i}, $chiper{j}, strlen({i}));\n"
                j += 1
            out = out.rstrip("\n")
            return out
        if n == "ARRAY":
            return a[0]
        if n == "MUTATE":
            return re.sub(r'params\[\d+\]\.memref\.buffer', o, a[0])
        if n == "RAW":
            return a[0]
        args = ", ".join(a)
        return (o + " = " if o else "") + f"{n}({args});"

    @staticmethod
    def emit_block(ir_list: List[TeeIRCall]) -> str:
        return "\n".join(CEmitter.emit(x) for x in ir_list)


RULE_ENCRYPT_BEFORE_OUTPUT = DslRule.parse(
    "COPY($out, $plain, $len)->_ => "
    "MALLOC($len)->$cipher; "
    "ENC($plain, $cipher, $len)->_; "
    "COPY($out, $cipher, $len)->_"
)

RULE_ENCRYPT_BEFORE_OUTPUT_SNPRINT = DslRule.parse(
    "SNPRINT($out, $len, $format, $argnum, $args)->_ => "
    "MULMALLOC($args)->$ciphers; "
    "MULENC($args)->_; "
    "SNPRINT($out, $len, $format, $argnum)->_"
)

RULE_VALIDATE_BEFORE_USE = DslRule.parse(
    # 用 IF 插入检查，再保留原 COPY
    "COPY($dst, $in, $n)->_ => "
    "IF($n, >, $v)->_; "
    "COPY($dst, $in, $n)->_"
)

RULE_VALIDATE_ARRAY_BEFORE_USE = DslRule.parse(
    # 用 IF 插入检查，再保留原 COPY
    "ARRAY($ar, $index)->_ => "
    "IF($index, >, $v)->_; "
    "IF($index, <, 0)->_; "
    "ARRAY($ar, $index)->_"
)

RULE_NO_SHALLOW_SHARED = DslRule.parse(
    # 读路径：把 SHALLOW(p)->x 改写为 deep copy + hash 校验
    "SHALLOW($sm)->$buf => "
    "MALLOC($size)->$buf; "
    "COPY($buf, $sm, $size)->_; "
    "READ()->$h1;"
    "HASH($h2, $buf,$size)->_; "
    "IF(equal($h1,$h2), !=, 0)->_"
)

RULE_MUTATE_SHARED = DslRule.parse(
    # 读路径：把 SHALLOW(p)->x 改写为 deep copy + hash 校验
    "MUTATE($v)->$sm=> "
    "MUTATE($v)->$buf;"
    "HASH($h, $buf, $size)->_; "
    "WRITE($h)->_;"
    "COPY($sm, $buf, $size)->_; "
)

def parse_c_function(line):
    pattern = r'(\w+)\s*\((.*)\)\s*;'
    match = re.match(pattern, line.strip())
    if not match:
        return None, []

    func_name = match.group(1)
    params_str = match.group(2).strip()

    params = []
    current = []
    depth = 0
    in_string = False
    escape = False

    for ch in params_str:
        if in_string:
            current.append(ch)
            if escape:
                escape = False
            elif ch == '\\':
                escape = True
            elif ch == '"':
                in_string = False
        else:
            if ch == '"':
                in_string = True
                current.append(ch)
            elif ch == ',' and depth == 0:
                params.append(''.join(current).strip())
                current = []
            else:
                if ch == '(':
                    depth += 1
                elif ch == ')':
                    depth -= 1
                current.append(ch)

    if current:
        params.append(''.join(current).strip())

    return func_name, params

def get_code_line(filename, line_number):
    with open(filename, 'r', encoding='utf-8') as f:
        for i, line in enumerate(f, start=1):
            if i == line_number:
                return line.strip()
    return None

def extract_first_bracket_content(line):
    """
    提取第一对外层 [] 的内容，支持嵌套 []
    """
    start = line.find('[')
    if start == -1:
        return None

    depth = 0
    content = ''
    for i in range(start, len(line)):
        ch = line[i]
        if ch == '[':
            depth += 1
        elif ch == ']':
            depth -= 1
            if depth == 0:
                return ''.join(content).strip()
        else:
            if depth >= 1:
                content += ch
    return None

def extract_number(s):
    # 提取所有数字
    nums = re.findall(r'\d+', s)
    if len(nums) >= 2:
        return int(nums[-2])  # 倒数第二个数字
    return None

def extract_info(s):
    # 提取所有数字
    info = s.split(',')
    return info[0], info[1]  # 返回第一个和第二个部分

def process_lists(lists, processed, i):
    """
    输入: 一个字典 {list_name: [list_of_strings]}
    输出: 排序后的 (数字, list_name, 原始字符串)
    """
    for s in lists:
        num = extract_number(s)
        if i == 1 or i == 2:
            info1, info2 = extract_info(s)
        if num is not None:
            if i == 0:
                processed.append((num, i))
            if i == 1:
                processed.append((num, i, info1, info2))
            if i == 2:
                processed.append((num, i, info1, info2))
    
    # 按数字排序
    processed.sort(key=lambda x: x[0])
    return processed

def process_file(filename):
    results = []

    with open(filename, 'r', encoding='utf-8') as f:
        while True:
            title = f.readline()
            if not title:  # 文件结束
                break
            title = title.strip()

            data_line = f.readline()
            if not data_line:  # 没有第二行了
                break

            try:
                # 把第二行字符串转成 Python 列表
                data_list = ast.literal_eval(data_line.strip())
            except Exception as e:
                data_list = []
                print(f"解析失败: {e}, 行内容: {data_line}")

            results.append(data_list)

    return results

def demo_encrypt_before_output(func_name, params) -> None:
    if func_name == 'TEE_MemMove':
        ir = [
            TeeIRCall("COPY", params)
        ]
        after, hits = rewrite(RULE_ENCRYPT_BEFORE_OUTPUT, ir)
    elif func_name == 'snprintf':
        new_params = []
        new_params.append(params[0])  # out
        new_params.append(params[1])  # len
        new_params.append(params[2])  # format
        new_params.append(len(params[3:]))
        new_params.append(params[3:])
        # print(new_params)
        ir = [
            TeeIRCall("SNPRINT", new_params)
        ]
        after, hits = rewrite(RULE_ENCRYPT_BEFORE_OUTPUT_SNPRINT, ir)
    else:
        return
    # print("[EncryptBeforeOutput] hits:", hits)
    # print(CEmitter.emit_block(after))
    return CEmitter.emit_block(after)

def demo_validate_before_use(func_name, params, original) -> None:
    if func_name == 'TEE_MemMove':
        ir = [
            TeeIRCall("COPY", params)
        ]
        after, hits = rewrite(RULE_VALIDATE_BEFORE_USE, ir)
    elif func_name == 'ARRAY':
        inside_brackets = extract_first_bracket_content(original)
        ir = [
            TeeIRCall("ARRAY", [original, inside_brackets])
        ]
        after, hits = rewrite(RULE_VALIDATE_ARRAY_BEFORE_USE, ir)
    else:
        return
    # print("[ValidateBeforeUse] hits:", hits)
    # print(CEmitter.emit_block(after))
    return CEmitter.emit_block(after)


def demo_no_shallow_shared(func_name, params, out) -> None:
    if func_name == 'SHALLOW':
        ir = [
            TeeIRCall("SHALLOW", [params], output=out)
        ]
        after, hits = rewrite(RULE_NO_SHALLOW_SHARED, ir)
    elif func_name == 'MUTATE':
        ir = [
            TeeIRCall("MUTATE", [line], output=f"params[{params}].memref.buffer"),
        ]
        after, hits = rewrite(RULE_MUTATE_SHARED, ir)
    else:
        return
    # print("[NoShallowShared] hits:", hits)
    # print(CEmitter.emit_block(after))
    return CEmitter.emit_block(after)

client = OpenAI(api_key="")
file_tokens = open("file_tokens.csv", "w")

def upload_file(path: str):
    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        return f.read()

def ask_about_file(question: str, model: str = "gpt-4.1-mini"):
    """
    基于已上传文件进行问答/总结。
    这里开启 file_search 工具，并把文件作为附件传给 Responses API。
    """
    resp = client.responses.create(
        model=model,
        input=[
        {
            "role": "user",
            "content": [
                # {
                #     "type": "input_text",
                #     "text": file_src,
                # },
                # {
                #     "type": "input_text",
                #     "text": history,
                # },
                {
                    "type": "input_text",
                    "text": question,
                },
            ]
        }
    ]
    )
    # Responses API 的文本结果通常在 output_text
    print("\n=== Model Answer ===\n")

    return resp.output_text, resp

def modify_file(file_path, changes, output_path=None):
    """
    根据行号修改文件内容，可支持替换、删除、插入。

    :param file_path: 原始文件路径
    :param changes: 一个字典，key为行号(从1开始)，value为要执行的操作
                    格式示例：
                    {
                        2: {"action": "replace", "text": "这是新内容"},
                        4: {"action": "delete"},
                        6: {"action": "insert", "text": "在第6行前插入内容"}
                    }
    :param output_path: 可选，修改后输出的文件路径，如果不提供则覆盖原文件
    """

    with open(file_path, "r", encoding="utf-8") as f:
        lines = f.readlines()

    new_lines = []
    for i, line in enumerate(lines, start=1):
        change = changes.get(i)
        if change:
            action = change["action"]
            if action == "replace":
                new_lines.append(change["text"] + "\n")
            elif action == "delete":
                continue  # 跳过此行
            elif action == "insert":
                new_lines.append(change["text"] + "\n")
                new_lines.append(line)  # 原始行保留
            else:
                raise ValueError(f"未知操作: {action}")
        else:
            new_lines.append(line)

    # 检查是否有行号超出文件范围的插入
    for ln, change in changes.items():
        if ln > len(lines) and change["action"] == "insert":
            new_lines.append(change["text"] + "\n")

    output_path = output_path or file_path
    with open(output_path, "w", encoding="utf-8") as f:
        f.writelines(new_lines)

if __name__ == "__main__":
    result = process_file("report")

    processed = []

    for i, lst in enumerate(result):
        processed = process_lists(lst, processed, i)
    print(processed)

    file_src = upload_file("./bad-partitioning/ta/entry.c")

    history = "Repair Record:\n"

    issues = ['Unencrypted Data Output', 'Input Validation Weaknesses ', 'Direct Usage of Shared Memory']

    # n_processed = processed[56:57]  # 按行号从大到小处理，避免修改代码后行号变化影响后续处理
    n_processed = processed
    print(n_processed)

    changes = {}

    for item in n_processed:
        start_time = datetime.now()

        if item[1] == 0:
            line = get_code_line("./bad-partitioning/ta/entry.c", item[0])
            if line:
                func_name, params = parse_c_function(line)
                if func_name and params:
                    resc = demo_encrypt_before_output(func_name, params)
            else:
                resc = None
        elif item[1] == 1:
            line = get_code_line("./bad-partitioning/ta/entry.c", item[0])
            if line and ("accesstoarray" not in item[3]):
                func_name, params = parse_c_function(line)
                if func_name and params:
                    resc = demo_validate_before_use(func_name, params, line)
            else:
                resc = demo_validate_before_use("ARRAY", item[2], line)
        elif item[1] == 2:
            line = get_code_line("./bad-partitioning/ta/entry.c", item[0])
            if line and ("initializerforbuf" in item[3]):
                tmp = line.split('=')
                resc = demo_no_shallow_shared("SHALLOW",tmp[1].strip().rstrip(';'), "$buf")
            elif "=" in line and (f"params[{item[2]}].memref.buffer" in line):
                resc = demo_no_shallow_shared("MUTATE", item[2], line)
            else:
                resc = line
        print(f"// Line {item[0]}: {line}")
        print(resc)

        file = f"""
We have a C code file with bad partitioning issues:
{file_src}
        """

        prompt_for_sm = """
You should also replace the fileds that use the shallow copy and shared memory parameters (e.g., params[1].memref.buffer) with the deep copy in the history repair.
        """

        prompt = f"""
New repair:
Following is a code snippet from the above code in line {item[0]}:
{line}
It has a bad partitioning issue: {issues[item[1]]}.
Repair the code with the following template code:
{resc}
You need to deduce and replace the fields starting with $ in the template based on the above code context and previous repairs, and avoid using variable names that have already been defined in the code or in history repair.
{prompt_for_sm if item[1] == 2 else ""}
Only output the repaired template code.
        """

        ques = file_src + "\n" + history + "\n" + prompt
        respon, respo = ask_about_file(ques)
        print(respon)
        replace_match = re.search(r'```c\s*([\s\S]*?)```', respon)
        replace_code = replace_match.group(1).strip() if replace_match else respon.strip()

        changes[item[0]] = {"action": "replace", "text": replace_code}

        history += f"Q: {prompt}\nA: {respon}\n"

        end_time = datetime.now()
        duration = (end_time - start_time).total_seconds()

        file_tokens.write(str(respo.usage.input_tokens) + ", " + str(respo.usage.output_tokens) + ", " + str(duration) + "\n")

    modify_file("./bad-partitioning/ta/entry.c", changes, "./bad-partitioning/ta/entry_r.c")

    # print("====== DEMO 1: Encrypt before output ======")
    # demo_encrypt_before_output()
    # print("\n====== DEMO 2: Validate before use ======")
    # demo_validate_before_use()
    # print("\n====== DEMO 3: No shallow shared ======")
    # demo_no_shallow_shared()
    # name, params = parse_c_function("snprintf(params[1].memref.buffer, params[1].memref.size, \"%s-%s-%s\", key, vi, s);")
