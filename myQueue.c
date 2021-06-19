#include "myQueue.h"

node_t *head = NULL;
node_t *tail = NULL;


void enqueue(int *obj, struct timeval* arrival_time){
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->client_fd = obj;
    new_node->next = NULL;
    new_node->arrival_time = *arrival_time;

    if (tail == NULL){
        head = new_node;
    }
    else{
        tail->next = new_node;
    }
    tail = new_node;
    size_of_queue++;
}

int* dequeue(struct timeval *arrival_time){
    if (head == NULL){
        return NULL;
    }else{
        int *result = head->client_fd;
        node_t *temp = head;
        *arrival_time = head->arrival_time;
        head = head->next;
        if (head == NULL){
            tail = NULL;
        }
        size_of_queue--;
        free(temp);
        return result;
    }
}
