#include "mgroup.h"

/* message queue(shared to all groups, because we need detect deadlock) */
/* message queue store proc queues */
static mqueue *msg_queue = NULL;        

static mgroup mgrp[NR_GRPS];            /* group table [this design is similar to proc design in minix] */
static int g_nr_ptr = 0;                /* group number ptr */
static int g_id_ctr = 1;                /* group id counter */
static mgroup *cur_group;               /* current message group*/

/******************************************************************
                        private methods prototype
 ********************************************************************/
// Group methods
int invalid(int strategy);                                  /* valid strategy */ 
int getgroup(int grp_nr, mgroup ** g_ptr);                  /* get group by its gid */

// Unblock
void try_unblock(mqueue *block_queue, mqueue *unblock_queue, int src);   /* try unblock a process */
void do_unblock(endpoint_t proc_e, message *msg);           /* unblock a process */
int do_server_unblock(mgroup *g_ptr, int src);              /* unblock procs */

// Process 
endpoint_t getendpoint(int proc_id);                        /* get endpoint from proc list*/
int getprocindex(mgroup *g_ptr, int proc);                  /* get proc index in group*/
int getprocqueue(endpoint_t proc_e, mqueue **proc_q);       /* get proc queue */

// Deadlock
int deadlock(mgroup *g_ptr, int call_nr);                   /* valid deadlock */ 
void deadlock_addpend(mqueue *proc_q, mqueue *pend_q, int call_nr);  /*recursive detect deadlock */

// Busy lock(reserved)
int acquire_lock(mgroup *g_ptr);                            /* simple busy lock. better solution: kernel call & spinlock / semaphore*/
int release_lock(mgroup *g_ptr);                            /* simple busy lock */

// Msend/receive
int send_pending(mgroup *g_ptr, message *msg, int src, int ipc_type);                           /* send enqueue pending queue */
int rec_pending(mgroup *g_ptr, message *msg, int src, int ipc_type);                            /* receive enqueue pending queue */


int do_opengroup()
{
    mgroup *g_ptr = NULL;
    int i, strategy;
    
    strategy = m_in.m1_i1;
    if(invalid(strategy)){                         // Make sure strategy is valid. 0 is allowed
        return EIVSTTG;                             // Invalid strategy. which defined in sys/errno.h
    }
    
    if(msg_queue == NULL){                          // Init message queue if it is null.
        initqueue(&msg_queue);
    }
    
    for(i=0; i<NR_GRPS; i++, g_nr_ptr++){
        g_nr_ptr %= NR_GRPS;                        // Circle detect.
        if(mgrp[g_nr_ptr].g_stat == M_UNUSED){      // This group is not used until now.
            g_ptr = &mgrp[g_nr_ptr];
            break;
        }
    }
    
    if(g_ptr == NULL){                              // No avalible(free) group in PM server.
        return EGRPBUSY;                            // Resource busy
    }
    g_ptr->g_stat = M_READY;
    g_ptr->g_nr = g_id_ctr;
    g_ptr->g_sttg = strategy;
    g_ptr->p_size = 0;
    g_ptr->lock = 0;
    initqueue(&g_ptr->valid_q);
    initqueue(&g_ptr->pending_q);
    initqueue(&g_ptr->invalid_q);
    g_id_ctr++;
    
    return g_ptr->g_nr;
}

int do_addproc(){
    mgroup *g_ptr = NULL;
    int grp_nr, proc_id;	
    endpoint_t proc_ep;
    
    grp_nr = m_in.m1_i1;
    proc_id = m_in.m1_i2;
    if(getgroup(grp_nr, &g_ptr) == -1){
        return EIVGRP;
    }else if(g_ptr->p_size >= NR_MGPROCS){
        return EPROCLEN;                    // reach max length
    }else if((proc_ep=getendpoint(proc_id))<0){
        return EIVPROC;
    }else if(getprocindex(g_ptr, proc_ep) != -1){
        return EPROCEXIST;                  // proc already exist
    }
    *(g_ptr->p_lst+g_ptr->p_size) = proc_ep;
    g_ptr->p_size++;
    return 0;
}

int do_rmproc(){
    mgroup *g_ptr = NULL;
    int i, grp_nr, proc_id;
    endpoint_t proc_ep;
    
    grp_nr = m_in.m1_i1;
    proc_id = m_in.m1_i2;
    if(getgroup(grp_nr, &g_ptr) == -1){
        return EIVGRP;
    } else if((proc_ep=getendpoint(proc_id))<0){
        return EIVPROC;
    } else if((i=getprocindex(g_ptr, proc_ep)) == -1){
        return ENOPROC;                     // cant find proc in group
    } 
    
    for(; i<g_ptr->p_size-1;i++){
        *(g_ptr->p_lst+i) = *(g_ptr->p_lst+i+1);
    }
    g_ptr->p_size--;
    return 0;
} 

int do_closegroup(){
    mgroup *g_ptr = NULL;
    int grp_nr;
    
    grp_nr = m_in.m1_i1;
    if(getgroup(grp_nr, &g_ptr) == -1){
        return EIVGRP;
    }
    g_ptr->g_stat = M_UNUSED;
    g_ptr->g_nr = 0;
    g_ptr->g_sttg = 0;
    g_ptr->p_size = 0;
    
    return 0;
}

int do_recovergroup(){
    mgroup *g_ptr = NULL;
    int grp_nr, strategy;
    void *value;
    
    grp_nr = m_in.m1_i1;
    strategy = m_in.m1_i2;
    if(invalid(strategy)){                           // Make sure strategy is valid. 0 is allowed
        return EIVSTTG;
    }else if(getgroup(grp_nr, &g_ptr) == -1){
        return EIVGRP;
    }
    g_ptr->g_stat -= M_DEADLOCK;                    // find out the method type.
    if(g_ptr->g_stat != M_SENDING || g_ptr->g_stat != M_RECEIVING){
        return -1;
    }
    
    switch(strategy){
        case IGNORE_ELOCK:
            // Clear all invalid chain.
            while(queue_func->dequeue(&value, g_ptr->invalid_q));
            do_server_unblock(g_ptr, g_ptr->g_stat-1);                  // SEND=1, M_SENDING=2, then 2-1 = SEND
            break;
        case CANCEL_IPC:
            while(queue_func->dequeue(&value, g_ptr->invalid_q));
            while(queue_func->dequeue(&value, g_ptr->pending_q));
            while(queue_func->dequeue(&value, g_ptr->valid_q));
            notify(g_ptr->flag);                                        //notify sender.
            break;
        case CLEAR_MSG:
            break;
        case CLEAR_ALL_MSG:
            //TODO
            break;
    }
    
    // Release lock
    g_ptr->g_stat = M_READY;
    release_lock(g_ptr);
    return 0;
}

int do_msend(){
    int rv, src, caller, grp_nr, ipc_type, i, s;
    message *msg;
    mgroup *g_ptr = NULL;
    grp_message *g_m, *msg_m;
    mqueue *proc_q;
    void *value;
    
    // Init and valid
    caller = m_in.m1_i1;
    grp_nr = m_in.m1_i2;
    ipc_type = m_in.m1_i3;
    msg = (message*) malloc(sizeof(message));
    if(getgroup(grp_nr, &g_ptr) == -1){
        return EIVGRP;
    } else if((src=getendpoint(caller)) < 0){
        return EIVPROC;
    } else if(getprocindex(g_ptr, src) == -1){
        return ENOPROC;
    }
    
    // Copy data from m_in.
    if ((message *) m_in.m1_p1 != (message *) NULL) {
        rv = sys_datacopy(who_e, (vir_bytes) m_in.m1_p1,
            PM_PROC_NR, (vir_bytes) msg, (phys_bytes) sizeof(message));
        if (rv != OK) return(rv);
    } 
    
    // add a new message by different ipc_type.
    cur_group = g_ptr;
    g_ptr->g_stat = M_SENDING;
    
//    acquire_lock(g_ptr);
    rv = send_pending(g_ptr, msg, src, ipc_type);
    
    // return value
    if(queue_func->isempty(g_ptr->pending_q)) return NOIPCOP;
    rv = deadlock(g_ptr, SEND);                                                        // detect deadlock
    if(rv == 0) rv = do_server_unblock(g_ptr, SEND);                                   // try unblock
    return rv == 0 ? SUSPEND : rv;
}

int do_mreceive(){
    int rv, src, caller, grp_nr, ipc_type, i, s;
    mgroup *g_ptr = NULL;
    grp_message *g_m, *msg_m;
    mqueue *proc_q;
    void *value;
    
    // Init and valid
    caller = m_in.m1_i1;
    grp_nr = m_in.m1_i2;
    ipc_type = m_in.m1_i3;
    if(getgroup(grp_nr, &g_ptr) == -1){
        return EIVGRP;
    } else if((src=getendpoint(caller)) < 0){
        return EIVPROC;
    } else if(getprocindex(g_ptr, src) == -1){
        return ENOPROC;
    }
    
    // receive a new message by different ipc_type.
    cur_group = g_ptr;
    g_ptr->g_stat = M_RECEIVING;
    
//    acquire_lock(g_ptr);
    rv = rec_pending(g_ptr, NULL, src, ipc_type);
    
    // return value
    if(queue_func->isempty(g_ptr->pending_q)) return NOIPCOP;
    rv = deadlock(g_ptr, RECEIVE);                                                       // detect deadlock
    if(rv == 0) rv = do_server_unblock(g_ptr, RECEIVE);
    return rv == 0 ? SUSPEND : rv;
}

/*  ========================= private methods ================================*/

int do_server_unblock(mgroup *g_ptr, int call_type){
    int rv=0, stat = 0;
    mqueue *proc_q, *block_queue, *unblock_queue;
    message *msg;
    void *value;
    grp_message *g_m, *msg_m;
    
    initqueue(&unblock_queue);
    initqueue(&block_queue);
    while(queue_func->dequeue(&value, g_ptr->valid_q)){
        g_m = (grp_message *)value;
        if(getprocqueue(g_m->sender, &proc_q) == -1){            // If not exist, then build proc queue
            initqueue(&proc_q);
            proc_q->number = g_m->sender;
            queue_func->enqueue(g_m, proc_q);
            queue_func->enqueue(g_m, block_queue);
            queue_func->enqueue(proc_q, msg_queue);
        } else {                                                // if exist proc in message queue
            queue_func->iterator(proc_q);
            while(queue_func->next(&value, proc_q)){            // find this message
                msg_m = (grp_message *)value;
                if(msg_m->call_nr == g_m->call_nr) continue;    // Only when callnr+callnr=send+receive, then goto next step;
                if(msg_m->receiver == g_m->receiver){
                    msg_m = msg_m->call_nr == SEND ? msg_m: g_m;
                    queue_func->enqueue(msg_m, unblock_queue);
                    queue_func->removeitem(proc_q);             // Proc queue remove this message.
                    stat = 1;
                    break;
                }
            }
            if(stat != 1) {                                       // If did not find match message
                queue_func->enqueue(g_m, proc_q); 
                queue_func->enqueue(g_m, block_queue);
            }
        }
    }
    try_unblock(block_queue, unblock_queue, call_type);
    closequeue(unblock_queue);
    closequeue(block_queue);
    return rv;
}


int getprocindex(mgroup *g_ptr, int proc){
    int i;
    
    for(i=0; i<g_ptr->p_size;i++){
        if(*(g_ptr->p_lst+i) == proc){
            return i;
        }
    }
    return -1;
}

int getgroup(int grp_nr, mgroup ** g_ptr){
    int i, k;
    
    if(grp_nr < 0 || grp_nr>=NR_GRPS){
        return -1;
    }
    
    for(i=0, k=g_nr_ptr; i<NR_GRPS; i++, k--){
        k=(k+NR_GRPS)%NR_GRPS;
        if(mgrp[k].g_stat != M_UNUSED && mgrp[k].g_nr == grp_nr){       // find the group in group table.
            (*g_ptr) = &mgrp[k];
            return 0;
        }
    }
    return -1;
}

int invalid(int strategy){
    if(strategy < 0 || strategy > 10) return -1;
    return 0;
}

endpoint_t getendpoint(int proc_id){
    register struct mproc *rmp;
    if(proc_id < 0){
        return -1;
    }
    for (rmp = &mproc[NR_PROCS-1]; rmp >= &mproc[0]; rmp--){ 
        if (!(rmp->mp_flags & IN_USE)) continue;
        if (proc_id > 0 && proc_id == rmp->mp_pid) return rmp->mp_endpoint;
    }
    return -1;
}

void try_unblock(mqueue *block_queue, mqueue *unblock_queue, int call_type){
    mqueue *proc_q;
    struct node *n;
    int send_num = 0, b_num, flag = 0;
    grp_message *g_m, *msg_m;
    void *value;
    
    /******************************* unblock condition ********************************
     * When SEND  sender: block_queue->size == 0
     *            receiver : in unblock queue
     * When RECEIVE sender: all message received
     *            receiver : in unblock queue
     * Did not implement other unblock condtions until now, because of deadline limit.
     * *********************************************************************************/
    while(queue_func->dequeue(&value, unblock_queue)){
        g_m = (grp_message *)value;
        switch(call_type){
            case SEND:
                do_unblock(g_m->receiver, g_m->msg);        // unblock receiver
                if(block_queue->size == 0 && flag == 0){    // only unblock sender once.
                    do_unblock(g_m->sender, g_m->msg);
                    flag = 1;
                }
                break;
            case RECEIVE:
                do_unblock(g_m->receiver, g_m->msg);        // unblock receiver
                if(getprocqueue(g_m->sender, &proc_q) > 0){ // unblock sender
                    for(n = proc_q->head; n != NULL; n=n->nextNode){
                        msg_m = (grp_message *)n->value;
                        if(msg_m->call_nr == RECEIVE || msg_m->receiver == g_m->receiver) continue;
                        send_num++;
                    }
                    if(send_num == 0){
                        do_unblock(g_m->sender, g_m->msg);
                    }
                }
        }
    }
}

void do_unblock(endpoint_t proc_e, message *msg){
    register struct mproc *rmp;
    int s, proc_nr;
    
    for (rmp = &mproc[NR_PROCS-1]; rmp >= &mproc[0]; rmp--){ 
        if (!(rmp->mp_flags & IN_USE)) continue;
        if(proc_e == rmp->mp_endpoint){
            if (pm_isokendpt(rmp->mp_endpoint, &proc_nr) != OK) {
                panic("handle_vfs_reply: got bad endpoint from VFS: %d", rmp->mp_endpoint);
            }
            memcpy(&rmp->mp_reply, msg, sizeof(message));
            setreply(proc_nr, OK);
            if ((rmp->mp_flags & (REPLY | IN_USE | EXITING)) == (REPLY | IN_USE)) {
              s=sendnb(rmp->mp_endpoint, &rmp->mp_reply);
              if (s != OK) {
                  printf("PM can't reply to %d (%s): %d\n", rmp->mp_endpoint, rmp->mp_name, s);
              }
              rmp->mp_flags &= ~REPLY;
            }
        }
    }
}

int deadlock(mgroup *g_ptr, int call_nr){
    grp_message *g_m;
    mqueue *proc_q, *valid_q, *pend_q;
    int rv = 0, deadlock, dest_e;
    void *value;
    
    /******************************* deadlock condition/algorithm **********************
     * 1. push receiver A into pend_q. 
     * 2. interative check pend_q. eg, dequeue A from pend_q and put A into valid_q
     * 3. if the value A send to another process, eg, BCD, then push receivers into pend_q.
     * 4. valid B, C, D each. put them into valid_q
     * 5. iterative execute step 2,3,4. until the pend_q is empty.
     * 6. check sender S, if s in valid_q, that means circle send/receive [deadlock].
     * 7. else, not deadlock.
     * 8. for multiple message, we put them in group->pending_q and valid each from step 1->7
     * *********************************************************************************/
    
    // Iterative valid pending_q
    while(queue_func->dequeue(&value, g_ptr->pending_q)){
        g_m = (grp_message *)value;
        initqueue(&valid_q);
        initqueue(&pend_q);
        deadlock = 0;
        
        if(getprocqueue(g_m->receiver, &proc_q) != -1){
            queue_func->enqueue((void*)g_m->receiver, pend_q);
            while(queue_func->dequeue(&value, pend_q)){
                dest_e = (int)value;
                queue_func->enqueue((void *)dest_e, valid_q);                       // Put cur process into already 
                if(getprocqueue(dest_e, &proc_q) != -1){
                    deadlock_addpend(proc_q, pend_q, call_nr);
                }
            }
            if(queue_func->hasvalue((void *)g_m->sender, valid_q)){                  // if sender exist in the valid_q
                g_ptr->flag = who_e;
                g_ptr->g_stat += M_DEADLOCK;                                          //Deadlock(add deadlock)
                deadlock = 1;
                rv = ELOCKED;                                                       
            }
        }
        
        if(!deadlock){
            queue_func->enqueue(g_m, g_ptr->valid_q);
        } else {
            printf("Deadlock occur: %d(endpoint)->%d(endpoint)\n", g_m->sender, g_m->receiver);         // Print deadlock
            queue_func->enqueue(g_m, g_ptr->invalid_q);                          //input invalid_queue
        }
        closequeue(valid_q);
        closequeue(pend_q);
    }
    
    return rv;
}

/*
 * Add all dest into pend_q
 */
void deadlock_addpend(mqueue *proc_q, mqueue *pend_q, int call_nr){
    grp_message *msg_m;
    void *value;
    
    // Put all receiver into pend_q from current proc.
    queue_func->iterator(proc_q);
    while(queue_func->next(&value, proc_q)){
        msg_m =(grp_message *)value;
        if(msg_m->call_nr != call_nr) continue;
        queue_func->enqueue((void *)msg_m->receiver, pend_q);
    }
}

int send_pending(mgroup *g_ptr, message *msg, int src, int ipc_type){
    int i, s;
    grp_message *g_m, *msg_m;
    mqueue *proc_q;
    void *value;
    
    switch(ipc_type){
        case SENDALL:
            for(i=0; i<g_ptr->p_size; i++){
                if(src != g_ptr->p_lst[i]){
                    g_m = (grp_message *)malloc(sizeof(grp_message));
                    g_m->group=g_ptr;
                    g_m->sender=src;
                    g_m->receiver=g_ptr->p_lst[i];
                    g_m->call_nr=SEND;
                    g_m->msg= msg;
                    g_m->ipc_type = ipc_type;
                    queue_func->enqueue(g_m, g_ptr->pending_q);
                }
            }
            break;
        case IPCTOREQ:
            if(getprocqueue(src, &proc_q) > -1){    //Only send when proc queue exist
                queue_func->iterator(proc_q);
                while(queue_func->next(&value, proc_q)){
                    msg_m = (grp_message *)value;
                    if(msg_m->call_nr == RECEIVE){
                        g_m = (grp_message *)malloc(sizeof(grp_message));
                        g_m->group=g_ptr;
                        g_m->sender=src;
                        g_m->receiver=msg_m->receiver;
                        g_m->call_nr=SEND;
                        g_m->msg= msg;
                        g_m->ipc_type = ipc_type;
                        queue_func->enqueue(g_m, g_ptr->pending_q);
                    }
                }
            }
            break;
        case IPCNONBLOCK:
            for(i=0; i<g_ptr->p_size; i++){
                if(src != g_ptr->p_lst[i]){
                    s = getprocqueue(g_ptr->p_lst[i], &proc_q);
                    if(s < 0 || proc_q->size == 0){
                        g_m = (grp_message *)malloc(sizeof(grp_message));
                        g_m->group=g_ptr;
                        g_m->sender=src;
                        g_m->receiver=g_ptr->p_lst[i];
                        g_m->call_nr=SEND;
                        g_m->msg= msg;
                        g_m->ipc_type = ipc_type;
                        queue_func->enqueue(g_m, g_ptr->pending_q);
                    }
                }
            }
            break;
        default: 
            if((s=getendpoint(ipc_type)) < 0) return s; 
            g_m = (grp_message *)malloc(sizeof(grp_message));
            g_m->group=g_ptr;
            g_m->sender=src;
            g_m->receiver=s;
            g_m->call_nr=SEND;
            g_m->msg= msg;
            g_m->ipc_type = ipc_type;
            queue_func->enqueue(g_m, g_ptr->pending_q);
    }
    return 0;
}


int rec_pending(mgroup *g_ptr, message *msg, int src, int ipc_type){
    int i, s;
    grp_message *g_m, *msg_m;
    mqueue *proc_q;
    void *value;
    
    switch(ipc_type){
        case RECANY:
            for(i=0; i<g_ptr->p_size; i++){
                if(src != g_ptr->p_lst[i] && getprocqueue(g_ptr->p_lst[i], &proc_q) > -1 && proc_q->size > 0){
                    queue_func->iterator(proc_q);
                    while(queue_func->next(&value, proc_q)){
                        msg_m = (grp_message *)value;
                        if(msg_m->call_nr == SEND && msg_m->receiver==src){
                            g_m = (grp_message *)malloc(sizeof(grp_message));
                            g_m->group=g_ptr;
                            g_m->receiver=src;
                            g_m->sender=g_ptr->p_lst[i];
                            g_m->call_nr=RECEIVE;                                    
                            queue_func->enqueue(g_m, g_ptr->pending_q);
                            return 0;
                        }
                    }
                }
            }
            return NOIPCOP;
        case IPCTOREQ:
            for(i=0; i<g_ptr->p_size; i++){
                if(src != g_ptr->p_lst[i] && getprocqueue(g_ptr->p_lst[i], &proc_q) > -1 && proc_q->size > 0){
                    queue_func->iterator(proc_q);
                    while(queue_func->next(&value, proc_q)){
                        msg_m = (grp_message *)value;
                        if(msg_m->call_nr == SEND && msg_m->receiver==src){
                            g_m = (grp_message *)malloc(sizeof(grp_message));
                            g_m->group=g_ptr;
                            g_m->receiver=src;
                            g_m->sender=g_ptr->p_lst[i];
                            g_m->call_nr=RECEIVE;                                    
                            queue_func->enqueue(g_m, g_ptr->pending_q);
                            break;
                        }
                    }
                }
            }
            break;
        default:
            if((s=getendpoint(ipc_type)) < 0) return s; 
            g_m = (grp_message *)malloc(sizeof(grp_message));
            g_m->group=g_ptr;
            g_m->receiver=src;
            g_m->sender=s;
            g_m->call_nr=RECEIVE;
            queue_func->enqueue(g_m, g_ptr->pending_q);
    }        
    return 0;
}

int getprocqueue(endpoint_t proc_e, mqueue **proc_q){
    void *value;
    mqueue *q;
    
    queue_func->iterator(msg_queue);
    while(queue_func->next(&value, msg_queue)){
        q = (mqueue *)value;
        if(q->number == proc_e){
            (*proc_q) = q;
            return 0;
        }
    }
    return -1;
}

int acquire_lock(mgroup *g_ptr){
    while(g_ptr->lock != 0);
    g_ptr->lock = 1;       // Enter critical region.
}

int release_lock(mgroup *g_ptr){
    g_ptr->lock = 0;
}
