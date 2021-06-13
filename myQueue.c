#include "myQueue.h"

node_t *head = NULL;
node_t *tail = NULL;


void enqueue(int *x){
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->client_fd = x;
    new_node->next = NULL;

    if (tail == NULL){
        head = new_node;
    }
    else{
        tail->next = new_node;
    }
    tail = new_node;
    size_of_queue++;
}

int* dequeue(){
    if (head == NULL){
        return NULL;
    }else{
        int *result = head->client_fd;
        node_t *temp = head;
        head = head->next;
        if (head == NULL){
            tail = NULL;
        }
        size_of_queue--;
        free(temp);
        return result;
    }
}
