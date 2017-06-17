#include <stdio.h>
#include <string.h>
#include <lib.h>    // provides _syscall and message
#include "minix/ipc.h"
#include "testhelper.h"

/**************************************************************
 *       test deadlock (send, send)
 *************************************************************/
int main()
{
    int gid, rv, child, parent=getpid();
    message m;
    gid=opengroup(0);
    ASSERT_GREATER(gid, 0);
    printf("You may need CTRL+C to exit block\n");

    if((child=fork())==0){
        // child
	rv = addproc(gid, getpid());
	ASSERT_EQUAL(rv, 0);
        m.m1_i1 = 10;
        rv = mreceive(gid, &m, parent);
	if(rv == -1){				// We do not sure which process(child? parent?) will cause deadlock
	   TEST_EQUAL(errno, ELOCKED, "parent receive success(blocked), and child deadlock");
	}
    } else {
        // This is parent
        rv = addproc(gid, parent);
	ASSERT_EQUAL(rv, 0);

        rv = mreceive(gid, &m, child);
	if(rv == -1){
	   TEST_EQUAL(errno, ELOCKED, "child receive success(blocked), and parent deadlock");
	}
    }
    
    closegroup(gid);
    return 0;
}
