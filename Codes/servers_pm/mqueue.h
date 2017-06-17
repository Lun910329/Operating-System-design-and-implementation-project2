#ifndef _MQUEUE_H_ 
#define _MQUEUE_H_ 

#define true 1
#define false 0

struct node{
    void *value;
    struct node *nextNode; 
};

typedef struct{  
    int size;      
    int number;
    struct node *head;   
    struct node *tail;  
    struct node *cur;
    struct node *prev;
    struct node *old_cur;
} mqueue;  

typedef int (*queue_f0)(mqueue *que);              /* only 1 item */       
typedef int (*queue_f1)(void *item, mqueue *que);  /* not need &return */       
typedef int (*queue_f2)(void **item, mqueue *que); 
typedef struct 
{  
    queue_f0 isempty;
    queue_f1 enqueue;
    queue_f2 dequeue;
    queue_f0 iterator;
    queue_f2 next;
    queue_f0 removeitem;
    queue_f1 hasvalue;
} queue_function;
queue_function *queue_func;

void initqueue(mqueue ** que);                
void closequeue(mqueue *que);
void printqueue(mqueue *que, char *name);
void printqueue2(mqueue *que, char *name);

#endif
