*************************** MINIX  VERSION3.2.1 **************************************
1.Source codes(because we modified servers and libs and sys etc. so you probably need download all minix from my github, because i am not sure whether you can compile correctly if somewhere missing.)
The whole image link is: https://drive.google.com/open?id=0Bw0d-ZkX59ijRmEwNWJSbFMwRUU

2. To compile minix, it may take serveral minutes to finish compile:
# cd /usr/src
# make build

3. Compile servers only(not suggested):
# cd /usr/src/releasetools
# make services
# make install

4. Modify logs(may not complete).
/usr/src/servers/pm/syscl_pext.c
/usr/src/include/lib_pext.h
/usr/src/include/syscl_errdef.h
<include/minix/callnr.h> 		*[modified : for system call numbers]
<include/minix/syslib.h>		*[reserved : for kernel calls]
<include/minix/com.h>			*[modified : for environment variables]
<include/unistd.h>			*[modified : for system calls prototypes]
<lib/libc/sys-minix/mgroup.c>		*[added for project2 methods]
<lib/libsys/sys_groupipc.c>		*[reserved for project2 kernel level err/deadlock detection]
<sys/sys/errno.h>			*[error definitions]
<servers/pm/main.c>			*[modified : to skip server auto ipc, and do my ipcs]
<servers/pm/mgroup.c>			*[added : for project2 server call methods]
<servers/pm/mqueue.h>			*[added : for message queue/deadlock detect and other queues]
<servers/pm/mqueue.c>
<servers/pm/proto.h>			*[modified : prototype of system calls]
<servers/pm/table.c>			*[modified : call numbers of system calls]	

5. Test cases           # /usr/src/2 
<mytest>     ---------------------	*[test cases]	
<mytest/testhelper.h> 			*[test helper, include TEST and ASSERT]
<mytest/testgroup.c>			*[test group methods]
<mytest/testipc.c>			*[test ipcs: who will not be blocked]
<mytest/testblockipc.c>			*[test ipcs: who will be blocked, you may need CTRL+C to exit]
<mytest/testdeadlock.c>			*[test deadlock:  may be blocked, you may need CTRL+C to exit]
# gcc [testfile.c] -o [testname]        compile test case
# ./[testname]        			execute test case