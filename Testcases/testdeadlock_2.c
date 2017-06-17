#include <stdio.h>
#include <string.h>
#include <lib.h>    
#include "minix/ipc.h"
#include "testhelper.h"


/**************************************************************
 *       test deadlock (A->B->C->D),  then D->A
 *************************************************************/
int main()
{
    message m;
    int status,i, pid[10], rv, parent=getpid();

    int gid = opengroup(0);
    ASSERT_GREATER(gid, 0);

    rv = addproc(gid, parent);
    ASSERT_EQUAL(rv, 0);

    printf("You may need CTRL+C to exit block\n");	
    
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
        //Wait parent send pid
        while(mreceive(gid, &m, parent)!=0);
	ASSERT_EQUAL(rv, 0);        
	ASSERT_EQUAL(m.m1_i2 - m.m1_i1, 3);

	if(i==3){
            i -= 4;
        }
        int rv = msend(gid, &m, m.m1_i1+i+1);		//Send to next, eg, pid[0] send to pid[1]
	if(rv == -1){
	    TEST_EQUAL(errno, ELOCKED, "deadlock occur and only occur once.");
            printf("deadlock from %d to %d\n", getpid(), m.m1_i1+i+1);
	} else {
	    printf("send from %d to %d success.", getpid(), m.m1_i1+i+1);
	}
	exit(0);	
    } else {
        //Parent proc 
	for(i=0; i<4; i++){
	    printf("child id %d\n", pid[i]);
	}
	m.m1_i1 = pid[0];
	m.m1_i2 = pid[3];   
        rv = msend(gid, &m, SENDALL);
	ASSERT_EQUAL(rv, 0);
    }
    return 0;
}