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

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <hello_world_ta.h>

#include <string.h>

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");

	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

void enc(char *plain, int len)
{
	for (int i = 0; i < len; i++) {
		plain[i] = plain[i] + 1;
	}
}

void output_p2(TEE_Param params[4])
{
	char key2[1000] = "12345678";
	char vi2[1000] = "abcdef";
	char s2[1000] = "vwxyz";

	if (params[1].memref.size < strlen(key2) + strlen(vi2) + strlen(s2) + 2)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (params[3].memref.size < strlen(key2) + 1)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (params[0].memref.size < strlen(key2) + 1)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	TEE_MemMove(params[3].memref.buffer, key2, strlen(key2));
	printf("%s\n", params[3].memref.buffer);
	
	snprintf(params[1].memref.buffer, params[1].memref.size, "%s", key2);
	printf("%s\n", params[1].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s", key2, vi2);
	printf("%s\n", params[1].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s-%s", key2, vi2, s2);
	printf("%s\n", params[1].memref.buffer);

	enc(key2, strlen(key2));

	TEE_MemMove(params[0].memref.buffer, key2, strlen(key2));
	printf("%s\n", params[0].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s", key2, vi2);
	printf("%s\n", params[1].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s-%s", key2, vi2, s2);
	printf("%s\n", params[1].memref.buffer);

	enc(vi2, strlen(vi2));

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s-%s", key2, vi2, s2);
	printf("%s\n", params[1].memref.buffer);
}

void output_p1(TEE_Param params[4])
{
	char key1[1000] = "1234567";
	char vi1[1000] = "abcde";
	char s1[1000] = "wxyz";

	if (params[1].memref.size < strlen(key1) + strlen(vi1) + strlen(s1) + 2)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (params[2].memref.size < strlen(key1) + 1)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (params[0].memref.size < strlen(key1) + 1)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	TEE_MemMove(params[2].memref.buffer, key1, strlen(key1));
	printf("%s\n", params[2].memref.buffer);
	
	snprintf(params[1].memref.buffer, params[1].memref.size, "%s", key1);
	printf("%s\n", params[1].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s", key1, vi1);
	printf("%s\n", params[1].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s-%s", key1, vi1, s1);
	printf("%s\n", params[1].memref.buffer);

	enc(key1, strlen(key1));
	TEE_MemMove(params[0].memref.buffer, key1, strlen(key1));
	printf("%s\n", params[0].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s", key1, vi1);
	printf("%s\n", params[1].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s-%s", key1, vi1, s1);
	printf("%s\n", params[1].memref.buffer);

	enc(vi1, strlen(vi1));

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s-%s", key1, vi1, s1);
	printf("%s\n", params[1].memref.buffer);

	output_p2(params);
}

static TEE_Result output(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	char key[1000] = "123456";
	char vi[1000] = "abcd";
	char s[1000] = "xyz";

	if (params[0].memref.size < strlen(key))
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (params[1].memref.size < strlen(key) + strlen(vi) + strlen(s) + 2)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}
		
	TEE_MemMove(params[0].memref.buffer, key, strlen(key));
	printf("%s\n", params[0].memref.buffer);
	
	snprintf(params[1].memref.buffer, params[1].memref.size, "%s", key);
	printf("%s\n", params[1].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s", key, vi);
	printf("%s\n", params[1].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s-%s", key, vi, s);
	printf("%s\n", params[1].memref.buffer);

	enc(key, strlen(key));
	TEE_MemMove(params[0].memref.buffer, key, strlen(key));
	printf("%s\n", params[0].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s", key, vi);
	printf("%s\n", params[1].memref.buffer);

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s-%s", key, vi, s);
	printf("%s\n", params[1].memref.buffer);

	enc(vi, strlen(vi));

	snprintf(params[1].memref.buffer, params[1].memref.size, "%s-%s-%s", key, vi, s);
	printf("%s\n", params[1].memref.buffer);

	output_p1(params);

	return TEE_SUCCESS;
}

void input_p2(int a, int b, char *buf1, int size1, char *buf2, int size2, char *buf3, int size3)
{
	char *str = TEE_Malloc(2048, 0);

	int tmp_arr1[30] = {0};
	int tmp_arr2[29] = {0};

	tmp_arr1[a] = 43;

	tmp_arr1[50 - a] = 43;

	tmp_arr2[b - 8] = 43;
	
	char *param1 = buf1;
	char value1 = param1[15];
	// int value2 = param1[25 - size1];

	memcpy(buf2, str, 2048);

	TEE_MemMove(str, buf3, size3);

	if (size2 < 2048)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	memcpy(buf2, str, 2048);

	TEE_MemMove(str, buf3, size3);

	TEE_Free(str);
}

void input_p1(int a, int b, char *buf1, int size1, char *buf2, int size2, char *buf3, int size3)
{
	char *str = TEE_Malloc(1024, 0);

	int tmp_arr1[25] = {0};
	int tmp_arr2[24] = {0};

	tmp_arr1[a] = 43;

	tmp_arr1[49 - a] = 43;

	tmp_arr2[b - 7] = 43;
	
	char *param1 = buf1;
	char value1 = param1[13];
	// int value2 = param1[20 - size1];

	memcpy(buf2, str, 1024);

	TEE_MemMove(str, buf3, size3);

	if (size2 < 1024)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	memcpy(buf2, str, 1024);

	TEE_MemMove(str, buf3, size3);

	TEE_Free(str);

	input_p2(a, b, buf1, size1, buf2, size2, buf3, size3);
}

void input_p(TEE_Param params[4])
{
	char str[4096] = "123456";

	int tmp_arr1[100] = {0};
	int tmp_arr2[99] = {0};

	tmp_arr1[params[0].value.a] = 43;

	tmp_arr2[params[0].value.b] = 43;
	
	char *param1 = params[1].memref.buffer;
	char value1 = param1[23];

	memcpy(params[2].memref.buffer, str, 4096);

	TEE_MemMove(str, params[3].memref.buffer, params[3].memref.size);
}

static TEE_Result input(uint32_t param_types,
	TEE_Param params[4])
{
	DMSG("has been called");

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
			TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_MEMREF_INPUT);

	if (param_types != exp_param_types) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	char *str = TEE_Malloc(1000, 0);

	int tmp_arr1[20] = {0};
	int tmp_arr2[19] = {0};

	tmp_arr1[params[0].value.a] = 43;

	tmp_arr1[50 - params[0].value.a] = 43;

	tmp_arr2[params[0].value.b - 8] = 43;
	
	int *param1 = params[1].memref.buffer;
	int value1 = param1[10];
	// int value2 = param1[15 - params[1].memref.size];

	memcpy(params[2].memref.buffer, str, 1000);

	TEE_MemMove(str, params[3].memref.buffer, params[3].memref.size);

	if (params[2].memref.size < 1000)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	memcpy(params[2].memref.buffer, str, 1000);

	TEE_MemMove(str, params[3].memref.buffer, params[3].memref.size);

	TEE_Free(str);

	input_p(params);

	input_p1(params[0].value.a, params[0].value.b, params[1].memref.buffer, params[1].memref.size, params[2].memref.buffer, params[2].memref.size, params[3].memref.buffer, params[3].memref.size);

	return TEE_SUCCESS;
}

void shared_memory_p2(TEE_Param params[4])
{
	int *buf = params[2].memref.buffer;
	uint32_t sz = params[2].memref.size;

	int value = buf[10];

	if (!TEE_MemCompare(params[2].memref.buffer, "123456", params[2].memref.size)) {
		IMSG("Pass!\n");
	}

	if (!TEE_MemCompare(buf, "123456", sz)) {
		IMSG("Pass!\n");
	}

	if (!strcmp(params[2].memref.buffer, "123456")) {
		IMSG("Pass!\n");
	}

	*((int *)params[2].memref.buffer + 10) = 0x55;

	buf[12] = 0x56;
}

void shared_memory_p1(TEE_Param params[4])
{
	int *buf = params[1].memref.buffer;
	uint32_t sz = params[1].memref.size;

	int value = buf[10];

	if (!TEE_MemCompare(params[1].memref.buffer, "123456", params[1].memref.size)) {
		IMSG("Pass!\n");
	}

	if (!TEE_MemCompare(buf, "123456", sz)) {
		IMSG("Pass!\n");
	}

	if (!strcmp(params[1].memref.buffer, "123456")) {
		IMSG("Pass!\n");
	}

	*((int *)params[1].memref.buffer + 10) = 0x55;

	buf[12] = 0x56;

	shared_memory_p2(params);
}

void shared_memory_p(int *buf, int size)
{
	// int value = buf[10];

	if (!TEE_MemCompare(buf, "123456", size)) {
		IMSG("Pass!\n");
	}

	if (!strcmp(buf, "123456")) {
		IMSG("Pass!\n");
	}

	*(buf + 10) = 0x55;
	buf[12] = 0x56;
}

static TEE_Result shared_memory(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	
	int *buf = params[0].memref.buffer;
	uint32_t sz = params[0].memref.size;

	int value = buf[10];

	if (!TEE_MemCompare(params[0].memref.buffer, "123456", params[0].memref.size)) {
		IMSG("Pass!\n");
	}

	if (!TEE_MemCompare(buf, "123456", sz)) {
		IMSG("Pass!\n");
	}

	if (!strcmp(params[0].memref.buffer, "123456")) {
		IMSG("Pass!\n");
	}

	*((int *)params[0].memref.buffer + 10) = 0x55;

	buf[12] = 0x56;

	shared_memory_p1(params);
	shared_memory_p(params[3].memref.buffer, params[3].memref.size);

	return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
	case TA_HELLO_WORLD_CMD_OUTPUT:
		return output(param_types, params);
	case TA_HELLO_WORLD_CMD_INPUT:
		return input(param_types, params);
	case TA_HELLO_WORLD_CMD_SHM:
		return shared_memory(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
