#include <stdio.h>
#include <string.h>
#include <lib.h>    
#include "minix/ipc.h"
#include "testhelper.h"


/**************************************************************
 *       test SENDALL to send all process in the group
 *************************************************************/
int main()
{
    message m1, m2, m3;
    int status,i, pid[10], rv, parent=getpid(), value[3];
    int gid1 = opengroup(0);
    int gid2 = opengroup(0);
    ASSERT_GREATER(gid1, 0);
    ASSERT_GREATER(gid2, gid1);
    
    rv = addproc(gid1, parent);
    ASSERT_EQUAL(rv, 0);
    rv = addproc(gid2, parent);
    ASSERT_EQUAL(rv, 0);

    for (i = 0; i < 10; i++){
        status = fork();
        if (status == 0 || status == -1) break;
        pid[i] = status;
	if(i<4){
            rv = addproc(gid1, pid[i]);
	} else {
	    rv = addproc(gid2, pid[i]);
	}
	ASSERT_EQUAL(rv, 0);
    }
    if (status == -1){
        //Fork error
    } else if (status == 0){
        //Child proc
	if(i<4){
            rv = mreceive(gid1, &m2, parent);   
	    ASSERT_EQUAL(rv, 0);
            TEST_EQUAL(m2.m1_i1, 10, "message should be 10.");
	} else {
            rv = mreceive(gid1, &m3, parent);   
	    ASSERT_EQUAL(rv, 0);
            TEST_EQUAL(m3.m1_i1, 20, "message should be 20.");
	} 	
	
    } else {
        //Parent proc   
	m1.m1_i1 = 10;
	rv = msend(gid1, &m1, SENDALL);
	ASSERT_EQUAL(rv, 0);
	TEST_EQUAL(rv, 0, "msend finish should return 0");

	m1.m1_i1 = 20;
	rv = msend(gid2, &m1, SENDALL);
	ASSERT_EQUAL(rv, 0);
	TEST_EQUAL(rv, 0, "msend finish should return 0");
    }
    return 0;
}