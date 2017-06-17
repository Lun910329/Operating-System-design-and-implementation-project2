#include "stdio.h"
#include "pm.h"
#include <sys/wait.h>
#include <assert.h>
#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/sched.h>
#include <minix/vm.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>   
#include "mproc.h"
#include "param.h"
#include "mqueue.h"

#ifndef _MGROUP_H_ 
#define _MGROUP_H_ 

#define NR_GRPS      256        /* max number of groups */
#define NR_MGPROCS   NR_PROCS   /* max number of processes in one group */

/* msg group state : similar to kernel process states. */
#define M_UNUSED       0        
#define M_READY        1  
#define M_SENDING      2      
#define M_RECEIVING    3 
#define M_DEADLOCK     4
#define M_ERROR        5 

/* msend/mreceive ipc type : IPC_TYPE, eg, msend(gid, &m, SENDALL); */
#define SENDALL        0            /* send to all other processes */
#define RECANY         0            /* receive any message when other processes send to current one*/
#define IPCNONBLOCK   -1            /* send/rec to unblock processes in current group*/ 
#define IPCTOREQ      -2            /* only send/receiver who requires this sender/receiver*/
/* We also can send/rec a single process by its pid: eg, msend(gid, &m, pid);          */ 

/* send/receive group block stategies: GROUP_STRATEGY, eg, opengroup(UB_ANY_REC) */
#define UB_ALL_REC    0             /* unblock sender when all receiver get this message */
#define UB_ANY_REC    1             /* unblock sender when any receiver get this message(did not finish) */
#define UB_ANY_CLEAR  2             /* when any receiver get this msg, then this message will be clear from message queue(did not finish) */

/* group recover stategies: RECOVER_STRATEGY, eg, recovergroup(IGNORE_ELOCK) */
#define IGNORE_ELOCK  0             /* ignore deadlock processes and continue send/receive to other processes */
#define CANCEL_IPC    1             /* cancel this ipc operation */
#define CLEAR_MSG     2             /* clear all messages in current group */
#define CLEAR_ALL_MSG 3             /* clear all messages in all groups */

#endif

typedef int grp_nr_t;           /* group number ptr */
typedef int strategy;           /* send/receive/recovery strategy */
typedef int grp_stat;           /* group state */

typedef struct{
    grp_nr_t g_nr;                  /* group number ptr */
    strategy g_sttg;                /* group strategy */
    int p_lst[NR_MGPROCS];          /* group processes */
    mqueue *valid_q;                /* valid message queue:   [store grp_message]*/
    mqueue *pending_q;              /* pending message queue: [store grp_message]*/
    mqueue *invalid_q;              /* invalid message queue:  deadlock queue */
    grp_stat g_stat;                /* group state */
    int p_size;                     /* process size */
    int flag;                       /* flag, reserved property */
    int lock;                       /* simple busy lock, may cause bug. Better solution: 1.kernel call & spinlock 2. semaphore
                                     *  but i dont have enough time to finish this.   */
}mgroup;

typedef struct{
    mgroup *group;                  /* group */
    endpoint_t sender;              /* sender */
    endpoint_t receiver;            /* receiver */
    int call_nr;                    /* caller_nr: SEND/RECEIVE */     
    int ipc_type;                   /* ipc type */
    message *msg;                   /* message */
}grp_message;