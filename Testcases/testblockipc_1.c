#include <stdio.h>
#include <string.h>
#include <lib.h>    
#include "testhelper.h"
#include "minix/ipc.h"


/**************************************************************
 *       test block
 *************************************************************/
int main()
{
    int gid, rv, child, parent=getpid();
    message m;
    
    gid=opengroup(0);
    ASSERT_GREATER(gid, 0);
    printf("You may need CTRL+C to exit block status\n");
    
    if((child=fork())==0){
        // child
        
    } else {
        // This is parent
        printf("Parent start receive %d\n", parent);
        rv = addproc(gid, parent);
        ASSERT_EQUAL(rv, 0);
	rv = addproc(gid, child);
        ASSERT_EQUAL(rv, 0);
        
        rv = mreceive(gid, &m, child);
        TEST_EQUAL(1, -1, "This will not be execute because this process is blocked");
    }
    return 0;
}
