#ifndef MYQUEUE_H_
#define MYQUEUE_H_

#include "segel.h"

// struct for keeping statistics for threads
struct thread_stats{
    int thread_id;
    int count_req;
    int static_req;
    int dynamic_req;
};
typedef  struct  thread_stats thread_stats_t;


int size_of_queue;
struct  node{
    int *client_fd;
    struct  node* next;

    struct timeval arrival_time;
    
};
typedef  struct  node node_t;

void enqueue(int *obj, struct timeval* arrival_time);
int* dequeue(struct timeval *arrival_time);
int* rand_dequeue();

#endif