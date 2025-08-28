import os

def parameter_type():
    params_list = []

    codeql_run = "codeql query run ..//query//parameter_type.ql " + \
            "--database=tee_example > temp//parameter_type.out"
    
    os.system(codeql_run)

    if os.path.exists("temp//parameter_type.out"):
        f = open("temp//parameter_type.out", "r")
        all_lines = f.readlines()
        for params_types in all_lines[2:]:
            tmp = params_types.replace(" ", "").split('|')
            params_list.append(tmp)

    return params_list


def switch_funcs():
    funcs = {}

    codeql_run = "codeql query run ..//query//cmd_id.ql " + \
            "--database=tee_example > temp//cmd_id.out"
    
    os.system(codeql_run)

    if os.path.exists("temp//cmd_id.out"):
        f = open("temp//cmd_id.out", "r")
        all_lines = f.readlines()
        for params_types in all_lines[2:]:
            tmp = params_types.replace(" ", "").split('|')
            funcs[tmp[1]] = tmp[2]
    
    return funcs


def uuid_list():
    uuid = [0] * 11

    codeql_run = "codeql query run ..//query//uuid.ql " + \
            "--database=tee_example > temp//uuid.out"
    
    os.system(codeql_run)

    if os.path.exists("temp//uuid.out"):
        f = open("temp//uuid.out", "r")
        all_lines = f.readlines()
        for params_types in all_lines[2:]:
            tmp = params_types.replace(" ", "").split('|')
            if tmp[3] == '0':
                if '{' not in tmp[1]:
                    uuid[int(tmp[2])] = int(tmp[1])
            if tmp[3] == '1':
                uuid[int(tmp[2]) + 3] = int(tmp[1])

    uuid_code = """
    TEEC_UUID uuid = {{ {}, {}, {}, \\
		{{ {}, {}, {}, {}, {}, {}, {}, {}}} }};""".format(uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], 
                                                   uuid[6], uuid[7], uuid[8], uuid[9], uuid[10])
    
    # print(uuid_code)
    
    return uuid_code


def list_gen(params_list, funcs):
    funcs_list = []
    for item in params_list:
        tmp_list = []
        for key in funcs:
            if item[5] in funcs[key]:
                tmp_list.append(key)
                for i in range(1, 5):
                    tmp_list.append(item[i])
                
                funcs_list.append(tmp_list)

    return funcs_list


def code_gen(i, func_params, uuid_code):
    
    func_code = 'int test{}(char *argv[])\n{{{}\n}}\n\n'
    
    op_code = ''
    for j in range(1, 5):
        ptype = int(func_params[j])
        if ptype >= 1 and ptype <= 3:
            op_code = op_code + """
    sscanf(argv[2], "%u", &op.params[{}].value.a);
    sscanf(argv[2], "%u", &op.params[{}].value.b);""".format(j - 1, j - 1)
        elif ptype != 0:
            op_code = op_code + """
    sscanf(argv[2], "%u", &op.params[{}].tmpref.size);
    void *buf{} = malloc(op.params[{}].tmpref.size);
    op.params[{}].tmpref.buffer = buf{};""".format(j - 1, j - 1, j - 1, j - 1, j - 1)
            
    print_code = ''
    for j in range(1, 5):
        ptype = int(func_params[j])
        if ptype == 6 or ptype == 7:
            print_code = print_code + """
    printf("out{}: %s\\n", op.params[{}].tmpref.buffer);""".format(j - 1, j - 1)
        elif ptype == 2 or ptype == 3:
            print_code = print_code + """
    printf("out{}a: %u\\n", op.params[{}].value.a);
    printf("out{}b: %u\\n", op.params[{}].value.b);""".format(j - 1, j - 1, j - 1, j - 1)
    # print(print_code)

    body_code = """
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    {}

    uint32_t err_origin;

    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
            res, err_origin);
    
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES({}, {}, {}, {});
    {}
    
    res = TEEC_InvokeCommand(&sess, {}, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    {}

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);""".format(uuid_code, func_params[1], func_params[2], func_params[3], func_params[4], op_code, func_params[0], print_code)

    test_code = func_code.format(i, body_code)
    # print(test_code)
    return test_code


params_list = parameter_type()
funcs = switch_funcs()
funcs_list = list_gen(params_list, funcs)
# print(funcs_list)
uuid_code = uuid_list()

head_code = '#include <err.h>\n#include <stdio.h>\n#include <string.h>\n#include <pthread.h>\n#include <unistd.h>\n#include <tee_client_api.h>\n\n'

i = 0
code = ''

main_body_code = ''

for item in funcs_list:
    test_code = code_gen(i, item, uuid_code)
    code = code + test_code
    main_body_code = main_body_code + '''
    test{}(argv);'''.format(i)
    i = i + 1

main_code = 'int main(int argc, char *argv[])\n{{{}\n}}\n\n'.format(main_body_code)

code = head_code + code + main_code

# print(code)

with open('..//test//test.c', 'w') as file:
    file.write(code)

make_file = """
TEE_SDK_DIR=$(shell pwd)
include ${TEE_SDK_DIR}/../config.mk

TEEC_EXPORT = ${TEE_SDK_DIR}/../optee_client/out/export

CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
READELF = $(CROSS_COMPILE)readelf

OBJS = test.o

CFLAGS += -Wall -I$(TEEC_EXPORT)/include
#Add/link other required libraries here
LDADD += -lteec -L$(TEEC_EXPORT)/lib -lpthread

BINARY = optee_test

.PHONY: all
all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) -o $@ $< $(LDADD)

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
"""

with open('..//test//Makefile', 'w') as file:
    file.write(make_file)
