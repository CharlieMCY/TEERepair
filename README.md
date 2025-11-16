# TEE Bad Partitioning Repair Tool

## Overview
This tool is designed to repair **bad partitioning issues** in Trusted Execution Environment (TEE) projects. It leverages **CodeQL** for static code analysis and uses the **gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf** cross-compilation toolchain for building TEE projects.

## Dependencies

- **CodeQL**: [Installation Guide](https://docs.github.com/en/code-security/codeql-cli/getting-started-with-the-codeql-cli/setting-up-the-codeql-cli).
- **gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf**: [Download](https://releases.linaro.org/components/toolchain/binaries/) and update `CROSS_COMPILE` in `config.mk` to cross-compiler path.
- **OP-TEE Prerequisites**: [Installation Guide](https://optee.readthedocs.io/en/latest/building/prerequisites.html).
- **PyCrypto**: pip3 install pycryptodome.

## Directory Structure

- **bad-partitioning**: Contains TEE projects used to evaluate the TEE bad partitioning repair tool.
- **optee_client** and **optee_os**: Dependencies required for building TEE projects.
- **query**: Includes CodeQL query scripts for analyzing TEE projects.
- **test**: Automatically generated test client.
- **dsl.py**: Code for DSL-basd repair.
- **test_driver_gen.py**: Code for automatically generating test client.

## Usage

### Step 1: Generate a report using the [bad partitioning detection tool](https://github.com/CharlieMCY/PartitioningE-in-TEE).
Save the results as the file `report`.
```bash
Unencrypted Data Output: 1
['file:///*/benchmark/Lenet5_in_OPTEE/ta/lenet5_ta.c:156:21:156:21']
Input Validation Weakness: 3
['accesstoarrayfile:///*/benchmark/Lenet5_in_OPTEE/ta/lenet.c:200:42:200:49', 'accesstoarrayfile:///*/benchmark/Lenet5_in_OPTEE/ta/lenet.c:280:25:280:33', 'accesstoarrayfile:///*/benchmark/Lenet5_in_OPTEE/ta/lenet.c:282:35:282:43']
Shared Memory Overwrite: 0
[]
```

### Step 2: Run repair tool
Configure `api_key` and run `dsl.py`.
