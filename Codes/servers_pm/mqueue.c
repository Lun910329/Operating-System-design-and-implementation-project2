#include "mqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "mgroup.h"

/*********************************************************
  * mqueue : a simple queue for project 2
  * Author : Yating Zhou(aka syscl)
  * Modified : Lun Li, Xuliang Zhang, Guangwei Ban
  * Version : 1.2
 *********************************************************/

/* private methods */
static int isempty(mqueue * que);
static int enqueue(void *item, mqueue * que);
static int dequeue(void **item, mqueue * que);
static int iterator(mqueue * que);
static int next(void **item, mqueue *que);
static int removeitem(mqueue *que);
static int hasvalue(void *item, mqueue * que);
queue_function que_function;


void initqueue(mqueue **que)
{
    if(queue_func == NULL || queue_func != &que_function){
        /* Bind methods */
        queue_func = &que_function;
        queue_func->isempty = isempty;
        queue_func->enqueue = enqueue;
        queue_func->dequeue = dequeue;
        queue_func->iterator = iterator;
        queue_func->next = next;
        queue_func->removeitem = removeitem;
        queue_func->hasvalue = hasvalue;
    }
    (*que) = (mqueue *)malloc(sizeof(mqueue));
    (*que)->head = NULL;
    (*que)->tail = NULL;
    (*que)->size = 0;
    return;
}
/*
enqueue item into queue
 */
static int enqueue(void *item, mqueue *que)
{
    struct node *newNode;
    newNode=(struct node *)malloc(sizeof(struct node));
    if(newNode==NULL)
    {
        return false;
    }
    else
    {
        newNode->value=item;
        newNode->nextNode = NULL;
        que->size++;
    }
    if(que->tail==NULL)
    {
        que->head=que->tail=newNode;
        que->cur = newNode;
    }else
    {
        que->tail=que->tail->nextNode=newNode;
    }
    return true;
}
/*
dequeue out first item from the queue
 */
static int dequeue(void **item, mqueue *que)
{
    struct node *p;
    if(isempty(que))
    {
        return false;
    }
    else
    {
        *item = que->head->value;
        p=que->head;
        que->head=p->nextNode;
        if(que->head==NULL)
        {
            que->tail=NULL;
        }
        que->size--;
        free(p);
        return true;
    }
}

static int iterator(mqueue * que){
    que->old_cur = que->cur;
    que->cur = que->head;
    que->prev = NULL;
}

static int next(void **item, mqueue *que){
	if(que->cur == NULL){
        (*item) = NULL;
        que->cur = que->old_cur;
        return false;
    }
    (*item) = que->cur->value;
    que->prev = que->cur;
    que->cur = que->cur->nextNode;
    return true;
}

static int removeitem(mqueue *que){
    struct node *n;
    if(que->prev == NULL){
        return false;
    } else if(que->prev == que->head){
        que->head = que->head->nextNode;
    } else {
        n = que->head;
        while (n->nextNode != que->prev) n=n->nextNode;
        n->nextNode = que->prev->nextNode;
    }
    if(que->prev == que->tail){
        que->tail = n;
    }
//    free(n);
    que->cur = NULL;
    que->size--;
    return true;
}

static int hasvalue(void *item, mqueue *que){
    struct node *p = que->head;
    while(p != NULL){
        if(p->value == item){
            return true;
        }
        p = p->nextNode;
    }
    return false;
}

/*
if queue is empty
 */
static int isempty(mqueue * que)
{
    if(que->head == NULL)
        return true;
    else
        return false;
}

void closequeue(mqueue *que){
    struct node *c, *p;

    c = que->head;
    while(c != NULL){
        p = c;
        c = c->nextNode;
        free(p);
    }
    free(que);
}

void printqueue(mqueue *que, char *name){
    struct node *c;

    if(name == NULL) name = "queue";
    printf("[%s items]: ", name);

    c = que->head;
    while(c != NULL){
        printf("%d ", (int)c->value);
        c = c->nextNode;
    }
    printf("\n");
}

void printqueue2(mqueue *que, char *name){
    struct node *c;
    grp_message *msg;

    if(name == NULL) name = "queue";
    printf("[%s items]: ", name);

    c = que->head;
    while(c != NULL){
        msg = (grp_message *)c->value;
        printf("%d->%d[%d] ", msg->sender, msg->receiver, msg->call_nr);
        c = c->nextNode;
    }
    printf("\n");
}
