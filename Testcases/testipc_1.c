#include <stdio.h>
#include <string.h>
#include <lib.h>    
#include "testhelper.h"
#include "minix/ipc.h"


/**************************************************************
 *       test msend/mreceive for single process
 *************************************************************/
int main()
{
    int gid, rv, child, parent=getpid();
    message m1, m2;
    
    gid=opengroup(0);
    ASSERT_GREATER(gid, 0);
    
    if((child=fork())==0){
        // child
        printf("Child start send %d\n", getpid());
        rv = addproc(gid, getpid());
        ASSERT_EQUAL(rv, 0);
        
        m1.m1_i1 = 10;
        rv = msend(gid, &m1, parent);
        
        TEST_EQUAL(rv, 0, "msend finish should return 0");
    } else {
        // This is parent
        printf("Parent start receive %d\n", parent);
        rv = addproc(gid, parent);
        ASSERT_EQUAL(rv, 0);
        
        rv = mreceive(gid, &m2, child);
        TEST_EQUAL(rv, 0, "mreceive finish should return 0");
        TEST_EQUAL(m2.m1_i1, 10, "receive value should be 10");
    }
    return 0;
}
