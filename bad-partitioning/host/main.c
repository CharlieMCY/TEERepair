/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* For the UUID (found in the TA's h-file(s)) */
#include <hello_world_ta.h>

#define TEST_BUFFER_SIZE	4096

char temp0[TEST_BUFFER_SIZE];
char temp1[TEST_BUFFER_SIZE];
char temp2[TEST_BUFFER_SIZE];
char temp3[TEST_BUFFER_SIZE];

int main(int argc, char *argv[])
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_HELLO_WORLD_UUID;
	uint32_t err_origin;

	TEEC_SharedMemory shared_mem0;
	TEEC_SharedMemory shared_mem1;
	TEEC_SharedMemory shared_mem2;
	TEEC_SharedMemory shared_mem3;

	uint32_t switch_id;
	
	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/*
	 * Open a session to the "hello world" TA, the TA will print "hello
	 * world!" in the log when the session is created.
	 */
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	/*
	 * Execute a function in the TA by invoking it, in this case
	 * we're incrementing a number.
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

	if (argc != 2 && argc != 4 && argc != 6) {
		printf("Error arguments: %d!\n", argc);
		return 0;
	}

	sscanf(argv[1], "%u", &switch_id);

	switch (switch_id)
	{
	case 0:
		/*
	 	 * Prepare the argument. Pass a value in the first parameter,
	 	 * the remaining three parameters are unused.
	 	 */
		op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_MEMREF_TEMP_INOUT,
				TEEC_MEMREF_TEMP_INOUT, TEEC_MEMREF_TEMP_INOUT);
		op.params[0].tmpref.buffer = temp0;
		op.params[0].tmpref.size = TEST_BUFFER_SIZE;

		op.params[1].tmpref.buffer = temp1;
		op.params[1].tmpref.size = TEST_BUFFER_SIZE;

		op.params[2].tmpref.buffer = temp2;
		op.params[2].tmpref.size = TEST_BUFFER_SIZE;

		op.params[3].tmpref.buffer = temp3;
		op.params[3].tmpref.size = TEST_BUFFER_SIZE;

		/*
		 * TA_HELLO_WORLD_CMD_INC_VALUE is the actual function in the TA to be
		 * called.
		 */
		res = TEEC_InvokeCommand(&sess, TA_HELLO_WORLD_CMD_OUTPUT, &op,
			&err_origin);
		if (res != TEEC_SUCCESS)
			errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
				res, err_origin);

		// printf("out0: %s\n", temp0);
		// printf("out1: %s\n", temp1);
		// printf("out2: %s\n", temp2);
		// printf("out3: %s\n", temp3);
		
		break;
	
	case 1:
		op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
		op.params[0].value.a = 40;

		op.params[1].tmpref.buffer = temp1;
		op.params[1].tmpref.size = TEST_BUFFER_SIZE;

		op.params[2].tmpref.buffer = temp2;
		op.params[2].tmpref.size = TEST_BUFFER_SIZE;

		op.params[3].tmpref.buffer = temp3;
		op.params[3].tmpref.size = TEST_BUFFER_SIZE;

		res = TEEC_InvokeCommand(&sess, TA_HELLO_WORLD_CMD_INPUT, &op,
			&err_origin);
		printf("input: 0x%x\n", res);
		
		break;
	
	case 2:
		op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INOUT, 
				TEEC_MEMREF_PARTIAL_INOUT, TEEC_MEMREF_PARTIAL_INOUT, TEEC_MEMREF_PARTIAL_INOUT);

		shared_mem0.size = 1000;
		shared_mem0.buffer = temp0;
		shared_mem0.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

		res = TEEC_RegisterSharedMemory(&ctx, &shared_mem0);
		if (res != TEEC_SUCCESS)
			errx(1, "TEEC_RegisterSharedMemory failed with code 0x%x",
				res);
					
		op.params[0].memref.parent = &shared_mem0;
		op.params[0].memref.offset = 0;
		op.params[0].memref.size = shared_mem0.size;

		shared_mem1.size = 1000;
		shared_mem1.buffer = temp1;
		shared_mem1.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

		res = TEEC_RegisterSharedMemory(&ctx, &shared_mem1);
		if (res != TEEC_SUCCESS)
			errx(1, "TEEC_RegisterSharedMemory failed with code 0x%x",
				res);
					
		op.params[1].memref.parent = &shared_mem1;
		op.params[1].memref.offset = 0;
		op.params[1].memref.size = shared_mem1.size;

		shared_mem2.size = 1000;
		shared_mem2.buffer = temp2;
		shared_mem2.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

		res = TEEC_RegisterSharedMemory(&ctx, &shared_mem2);
		if (res != TEEC_SUCCESS)
			errx(1, "TEEC_RegisterSharedMemory failed with code 0x%x",
				res);
					
		op.params[2].memref.parent = &shared_mem2;
		op.params[2].memref.offset = 0;
		op.params[2].memref.size = shared_mem2.size;

		shared_mem3.size = 1000;
		shared_mem3.buffer = temp3;
		shared_mem3.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

		res = TEEC_RegisterSharedMemory(&ctx, &shared_mem3);
		if (res != TEEC_SUCCESS)
			errx(1, "TEEC_RegisterSharedMemory failed with code 0x%x",
				res);
					
		op.params[3].memref.parent = &shared_mem3;
		op.params[3].memref.offset = 0;
		op.params[3].memref.size = shared_mem3.size;
		
		res = TEEC_InvokeCommand(&sess, TA_HELLO_WORLD_CMD_SHM, &op,
				&err_origin);
		
		if (res != TEEC_SUCCESS)
			errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
				res, err_origin);
			
		TEEC_ReleaseSharedMemory(&shared_mem1);

	default:
		break;
	}	

	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 * The TA will print "Goodbye!" in the log when the
	 * session is closed.
	 */

	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);

	return 0;
}