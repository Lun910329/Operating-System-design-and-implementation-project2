#include <stdio.h>
#include <stdlib.h>
#include "sys/errno.h"
#include <unistd.h>

#define TRUE 1
#define FALSE 0

static inline int ASSERT_EQUAL(int result, int expect)
{
    if(result == expect){
        return TRUE;
    }
    printf("ASSERT_EQUAL [%d==%d] Fail.\n", result, expect);
    exit(0);
}

static inline int ASSERT_GREATER(int result, int expect)
{
    if(result > expect){
        return TRUE;
    }
    printf("ASSERT_EQUAL [%d==%d] Fail.\n", result, expect);
    exit(0);
}

static inline void TEST_EQUAL(int result, int expect, char *msg){
    if(result == expect){
        printf("TEST_EQUAL [%s] OK.\n", msg);
    } else {
        printf("TEST_EQUAL [%s] Error, expect %d, result %d, errno %d\n", msg, expect, result, errno);
        exit(0);
    }
}

static inline void TEST_GREATER(int val1, int val2, char *msg){
    if(val1 > val2){
        printf("TEST_GREATER [%s] OK.\n", msg);
    } else {
        printf("TEST_GREATER [%s] Error, value1 %d, value2 %d, errno %d\n", msg, val1, val2, errno);
        exit(0);
    }
}