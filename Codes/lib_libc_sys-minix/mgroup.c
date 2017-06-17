#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>
#include <unistd.h>
#include <string.h>

int opengroup(int strategy){
    message m;
    
    m.m1_i1 = strategy;
    return _syscall(PM_PROC_NR, OPENGP, &m);
}

int addproc(int grp_nr, int proc){
    message m;
    
    m.m1_i1 = grp_nr;
    m.m1_i2 = proc;
    return _syscall(PM_PROC_NR, ADDPROC, &m);
}

int rmproc(int grp_nr, int proc){
    message m;
    
    m.m1_i1 = grp_nr;
    m.m1_i2 = proc;
    return _syscall(PM_PROC_NR, RMPROC, &m);
}

int closegroup(int grp_nr){
    message m;
    
    m.m1_i1 = grp_nr;
    return _syscall(PM_PROC_NR, CLOSEGP, &m);
}

int recovergroup(int grp_nr, int strategy){
    message m;
    
    m.m1_i1 = grp_nr;
    m.m1_i2 = strategy;
    return _syscall(PM_PROC_NR, RECOVERGP, &m);
}

int msend(int dest, void *msg, int type){
    message m;
    
    m.m1_i1 = getpid();
    m.m1_i2 = dest;
    m.m1_i3 = type;
    m.m1_p1 = (char*)msg;
   
    return _syscall(PM_PROC_NR, MSEND, &m);
}

int mreceive(int src, void *msg, int type){
    message m;
    int rv;
    
    m.m1_i1 = getpid();
    m.m1_i2 = src;
    m.m1_i3 = type;
    m.m1_p1 = (char*)msg;
    
    rv = _syscall(PM_PROC_NR, MRECEIVE, &m);
    memcpy(msg, &m, sizeof(message));
    return rv;
}


