#include <lib.h>    
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "testhelper.h"

/* test cases */
int test_opengroup_syscall();
int test_opengroup();
int test_opengroup_EIVSTTG();

int test_closegroup_syscall();
int test_closegroup();   
int test_closegroup_EIVGRP();

int test_addproc_syscall();
int test_addproc();   
int test_addproc_EIVGRP();
int test_addproc_EPROCEXIST();

int test_rmproc_syscall();
int test_rmproc();   
int test_rmproc_EIVGRP();
int test_rmproc_EIVPROC();

int main(void) {
    test_opengroup_syscall();
    test_opengroup();
    test_opengroup_EIVSTTG();

    test_closegroup_syscall();
    test_closegroup(); 
    test_closegroup_EIVGRP();

    test_addproc_syscall();
    test_addproc();  
    test_addproc_EIVGRP();
    test_addproc_EPROCEXIST();

    test_rmproc_syscall();
    test_rmproc();  
    test_rmproc_EIVGRP();
    test_rmproc_EIVPROC();
}

/* 
Test OPENGP syscall and Opengroup
*/
int test_opengroup_syscall(){
    message m;  
    int i;

    m.m1_i1 = 0;
 
    /* group id start from 0 */
    TEST_GREATER(_syscall(PM_PROC_NR, OPENGP, &m), -1, "test_open_syscall: opengroup should return id bigger than -1");
}

int test_opengroup(){
    message m;  
    int gid1, gid2, rv;
    
    gid1 = opengroup(0);
    gid2 = opengroup(0);

    TEST_GREATER(gid1, -1, "test_open: opengroup should return id bigger than -1");        
    TEST_GREATER(gid2, gid1, "test_open: new group id > old group id");     
    rv = closegroup(gid1);
    rv = closegroup(gid2);
}

/* 
Test Opengroup Exception Conditions
*/
int test_opengroup_EIVSTTG(){
    message m;  
    int rv;

    rv=opengroup(-1);
    TEST_EQUAL(rv, -1, "test_opengroup_EIVSTTG: test opengourp with invalid strategy should return -1");
    TEST_EQUAL(errno, EIVSTTG, "test_opengroup_EIVSTTG: test opengourp with invalid strategy errno should be EIVSTTG");
}

/* 
Test CLOSEGP syscall and Closegroup
*/
int test_closegroup_syscall(){
    message m;   
    int gid;

    gid = opengroup(0);
    ASSERT_GREATER(gid, 0);
    m.m1_i1 = gid;
 
    TEST_EQUAL(_syscall(PM_PROC_NR, CLOSEGP, &m), 0, "test_close_syscall: closegroup should return 0");
}

int test_closegroup(){
    message m; 
    int gid;
    int rv;

    gid = opengroup(0);

    rv = closegroup(gid);

    TEST_EQUAL(rv, 0, "test_close: closegroup should return 0");         
}

/* 
Test Closegroup Exception Conditions
*/
int test_closegroup_EIVGRP(){    
    message m; 
    int gid;
    int rv;

    gid = opengroup(0);

    rv = closegroup(-1);
    TEST_EQUAL(rv, -1, "test_close_EIVGRP: closegroup with invalid gid should return 0");    
    TEST_EQUAL(errno, EIVGRP, "test_close_EIVGRP: closegroup with invalid gid should return 0");    
}

/* 
Test ADDPROC syscall and Addprococess
*/
int test_addproc_syscall(){
    message m;  
    int gid;
    int child, parent=getpid();

    gid = opengroup(0);

    if((child=fork())==0){ 
        // in child process
        m.m1_i1 = gid;
        m.m1_i2 = getpid();
        TEST_EQUAL(_syscall(PM_PROC_NR, ADDPROC, &m), 0, "test_addprocess_syscall: addproc should return 0");
        exit(0);
    } else {
        // in parent process
		m.m1_i1 = gid;
        m.m1_i2 = parent;
        TEST_EQUAL(_syscall(PM_PROC_NR, ADDPROC, &m), 0, "test_addprocess_syscall: addproc should return 0");
    }
}

int test_addproc(){
    message m; 
    int gid;
    int child, parent=getpid();
    int rv;
    
    gid = opengroup(0);

    if((child=fork())==0){
        // in child process
        rv = addproc(gid, getpid());    
        TEST_EQUAL(rv, 0, "test_addprocess: addproc should return 0");  
        exit(0);
    } else {
        // in parent process
		rv = addproc(gid, parent);    
        TEST_EQUAL(rv, 0, "test_addprocess: addproc should return 0");
    }
    return 0;
}

/* 
Test Addprococess Exception Conditions
*/
int test_addproc_EIVGRP(){
    message m; 
    int gid;
    int child, parent=getpid();
    int rv;
    
    gid = opengroup(0);

    if((child=fork())==0){
        // in child process
        rv = addproc(-1, getpid());    
        ASSERT_EQUAL(rv, -1); 
        TEST_EQUAL(errno, EIVGRP, "test_addprocess_EIVGRP: addproc with invalid gid should return EIVGRP");
        exit(0);
    } else {
        // in parent process
		rv = addproc(-1, parent);    
        ASSERT_EQUAL(rv, -1); 
        TEST_EQUAL(errno, EIVGRP, "test_addprocess_EIVGRP: addproc with invalid gid should return EIVGRP");
    }
    return 0;
}
		
int test_addproc_EPROCEXIST(){
    message m; 
    int gid;
    int child, parent=getpid();
    int rv;
    
    gid = opengroup(0);
    
    rv = addproc(gid, getpid()); 
    ASSERT_EQUAL(rv, 0);
    
    rv = addproc(gid, getpid());    
    ASSERT_EQUAL(rv, -1); 
    TEST_EQUAL(errno, EPROCEXIST, "test_addprocess_EPROCEXIST: addproc with exist pid should return EPROCEXIST");
    return 0;
}

/* 
Test RMPROC syscall and rmproc
*/
int test_rmproc_syscall(){
    message m;  
    int gid;
    int child, parent=getpid();

    gid = opengroup(0);

    if((child=fork())==0){ 
        // in child process
        addproc(gid, getpid());
        m.m1_i1 = gid;
        m.m1_i2 = getpid();
        TEST_EQUAL(_syscall(PM_PROC_NR, RMPROC, &m), 0, "test_removeprocess_syscall: rmproc should return 0");
        exit(0);
    } else {
        // in parent process
		addproc(gid, parent);
        m.m1_i1 = gid;
        m.m1_i2 = parent;
        TEST_EQUAL(_syscall(PM_PROC_NR, RMPROC, &m), 0, "test_removeprocess_syscall: rmproc should return 0");
    }
    return 0;
}

int test_rmproc(){
    message m;  
    int gid, rv;
    int child, parent=getpid();

    gid = opengroup(0);
    if((child=fork())==0){ 
        // in child process
        addproc(gid, getpid());
        rv = rmproc(gid, getpid());
        TEST_EQUAL(rv, 0, "test_removeprocess: rmproc should return 0");
        exit(0);
    } else {
        // in parent process
		addproc(gid, parent);
        rv = rmproc(gid, parent);
        TEST_EQUAL(rv, 0, "test_removeprocess: rmproc should return 0");
    }
    return 0;
}

/* 
Test Removeprocess Exception Conditions
*/
int test_rmproc_EIVGRP(){
    message m;  
    int gid, rv;
    int child, parent=getpid();

    gid = opengroup(0);
    // in child process
    addproc(gid, getpid());
    rv = rmproc(-1, getpid());
    ASSERT_EQUAL(rv, -1);
    TEST_EQUAL(errno, EIVGRP, "test_removeprocess: rmproc with invalid gid should return EIVGRP");
    return 0;
}

int test_rmproc_EIVPROC(){
    message m;  
    int gid, rv;
    int child, parent=getpid();

    gid = opengroup(0);
    // in child process
    addproc(gid, getpid());
    rv = rmproc(gid, -1);
    ASSERT_EQUAL(rv, -1);
    TEST_EQUAL(errno, EIVPROC, "test_removeprocess: rmproc with invalid pid should return EIVPROC");
    return 0;
}