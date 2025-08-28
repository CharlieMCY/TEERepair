import os
import subprocess

test_list = [
    ['0'],
    ['1', '10000', '100', '100', '100'],
    ['2', '123456', '123456']
]

for item in test_list:
    test_cmd = ['sudo', 'optee_example_hello_world'] + item
    print(test_cmd)
    
    result = subprocess.run(test_cmd, capture_output=True, text=True)
    print(result.stdout)

