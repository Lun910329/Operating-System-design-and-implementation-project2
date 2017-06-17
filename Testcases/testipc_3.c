#include <stdio.h>
#include <string.h>
#include <lib.h>    
#include "minix/ipc.h"
#include "testhelper.h"


int main()
{
    message m1, m2;
    int status,i, pid[10], rv, parent=getpid();
    int gid = opengroup(0);
    ASSERT_GREATER(gid, 0);

    rv = addproc(gid, parent);
    ASSERT_EQUAL(rv, 0);

    for (i = 0; i < 4; i++){
        status = fork();
        if (status == 0 || status == -1) break;
        pid[i] = status;
        rv = addproc(gid, pid[i]);
        ASSERT_EQUAL(rv, 0);
    }
    if (status == -1){
        //Fork error
    } else if (status == 0){
        //Child proc
	if(i != 2){
            rv = mreceive(gid, &m2, parent);    
            TEST_EQUAL(m2.m1_i2, 99, "This may occur 0->3 times, and receive message 99.");	
	}
    } else {
        //Parent proc    
        printf("this is parent, cur id:%d\n", parent);
    	m1.m1_i2 = 99;
        rv = msend(gid, &m1, IPCTOREQ);
	ASSERT_EQUAL(rv, 0);
        rv = closegroup(gid);
    }
    return 0;
}