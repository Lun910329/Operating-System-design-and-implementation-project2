#include "syslib.h"

//int sys_ipcerrdtct(int function, endpoint_t src_e, endpoint_t dest_e, char *msg)
int sys_ipcerrdtct(int function, endpoint_t src_e, endpoint_t dest_e)
{
	int r;
    message m, msg;
    
    m.m1_i1 = function;
    m.m1_i2 = src_e;
    m.m1_i3 = dest_e;
    return (_kernel_call(SYS_IPCERRDTCT, &m));
}

int sys_singleipc(endpoint_t caller_e, endpoint_t src_dest_e, int call_nr, message *msg)
{
	int r;
    message m;
    
    m.m1_i1 = caller_e;
    m.m1_i2 = src_dest_e;
    m.m1_i3 = call_nr;
    m.m1_p1 = (char *)msg;
    return (_kernel_call(SYS_SINGLEIPC, &m));
}

int sys_groupipc(int function, endpoint_t src_e, endpoint_t dest_e)
{
    /*
	int r;
    message m, msg;
    
    m.m1_i1 = dest_e;
    m.m1_p1 = msg;
    m.m_type = ((message*)msg).m_type;
    m.m_source = src_e;
    return (_kernel_call(SYS_IPCERRDTCT, &m));
     */
     return 0;
}