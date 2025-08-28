#include <err.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <tee_client_api.h>

int test0(char *argv[])
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    
    TEEC_UUID uuid = { 2326458880, 9296, 4580, \
		{ 171, 226, 0, 2, 165, 213, 197, 27} };

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
    op.paramTypes = TEEC_PARAM_TYPES(1, 0, 0, 0);
    
    sscanf(argv[2], "%u", &op.params[0].value.a);
    sscanf(argv[2], "%u", &op.params[0].value.b);
    
    res = TEEC_InvokeCommand(&sess, 1, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
}

int test1(char *argv[])
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    
    TEEC_UUID uuid = { 2326458880, 9296, 4580, \
		{ 171, 226, 0, 2, 165, 213, 197, 27} };

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
    op.paramTypes = TEEC_PARAM_TYPES(7, 3, 0, 0);
    
    sscanf(argv[2], "%u", &op.params[0].tmpref.size);
    void *buf0 = malloc(op.params[0].tmpref.size);
    op.params[0].tmpref.buffer = buf0;
    sscanf(argv[2], "%u", &op.params[1].value.a);
    sscanf(argv[2], "%u", &op.params[1].value.b);
    
    res = TEEC_InvokeCommand(&sess, 2, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    
    printf("out0: %s\n", op.params[0].tmpref.buffer);
    printf("out1a: %u\n", op.params[1].value.a);
    printf("out1b: %u\n", op.params[1].value.b);

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
}

int test2(char *argv[])
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    
    TEEC_UUID uuid = { 2326458880, 9296, 4580, \
		{ 171, 226, 0, 2, 165, 213, 197, 27} };

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
    op.paramTypes = TEEC_PARAM_TYPES(3, 7, 7, 7);
    
    sscanf(argv[2], "%u", &op.params[0].value.a);
    sscanf(argv[2], "%u", &op.params[0].value.b);
    sscanf(argv[2], "%u", &op.params[1].tmpref.size);
    void *buf1 = malloc(op.params[1].tmpref.size);
    op.params[1].tmpref.buffer = buf1;
    sscanf(argv[2], "%u", &op.params[2].tmpref.size);
    void *buf2 = malloc(op.params[2].tmpref.size);
    op.params[2].tmpref.buffer = buf2;
    sscanf(argv[2], "%u", &op.params[3].tmpref.size);
    void *buf3 = malloc(op.params[3].tmpref.size);
    op.params[3].tmpref.buffer = buf3;
    
    res = TEEC_InvokeCommand(&sess, 0, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    
    printf("out0a: %u\n", op.params[0].value.a);
    printf("out0b: %u\n", op.params[0].value.b);
    printf("out1: %s\n", op.params[1].tmpref.buffer);
    printf("out2: %s\n", op.params[2].tmpref.buffer);
    printf("out3: %s\n", op.params[3].tmpref.buffer);

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
}

int test3(char *argv[])
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    
    TEEC_UUID uuid = { 2326458880, 9296, 4580, \
		{ 171, 226, 0, 2, 165, 213, 197, 27} };

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
    op.paramTypes = TEEC_PARAM_TYPES(0, 5, 0, 0);
    
    sscanf(argv[2], "%u", &op.params[1].tmpref.size);
    void *buf1 = malloc(op.params[1].tmpref.size);
    op.params[1].tmpref.buffer = buf1;
    
    res = TEEC_InvokeCommand(&sess, 1, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
}

int test4(char *argv[])
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    
    TEEC_UUID uuid = { 2326458880, 9296, 4580, \
		{ 171, 226, 0, 2, 165, 213, 197, 27} };

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
    op.paramTypes = TEEC_PARAM_TYPES(0, 0, 7, 0);
    
    sscanf(argv[2], "%u", &op.params[2].tmpref.size);
    void *buf2 = malloc(op.params[2].tmpref.size);
    op.params[2].tmpref.buffer = buf2;
    
    res = TEEC_InvokeCommand(&sess, 1, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    
    printf("out2: %s\n", op.params[2].tmpref.buffer);

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
}

int test5(char *argv[])
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    
    TEEC_UUID uuid = { 2326458880, 9296, 4580, \
		{ 171, 226, 0, 2, 165, 213, 197, 27} };

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
    op.paramTypes = TEEC_PARAM_TYPES(0, 0, 0, 5);
    
    sscanf(argv[2], "%u", &op.params[3].tmpref.size);
    void *buf3 = malloc(op.params[3].tmpref.size);
    op.params[3].tmpref.buffer = buf3;
    
    res = TEEC_InvokeCommand(&sess, 1, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
}

int main(int argc, char *argv[])
{
    test0(argv);
    test1(argv);
    test2(argv);
    test3(argv);
    test4(argv);
    test5(argv);
}

