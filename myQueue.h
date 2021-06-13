#include "segel.h"
#ifndef MYQUEUE_H_
#define MYQUEUE_H_


int size_of_queue;
struct  node{
    struct  node* next;
    int *client_fd;
    
};
typedef  struct  node node_t;

void enqueue(int* object);
int* dequeue();

#endif