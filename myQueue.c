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
        int *fd_of_deleted_client = head->client_fd;
        node_t *node_to_delete = head;
        *arrival_time = head->arrival_time;
        head = head->next;
        if (head == NULL){
            tail = NULL;
        }
        size_of_queue--;
        free(node_to_delete);
        return fd_of_deleted_client;
    }
}

int* rand_dequeue(){
    if (head == NULL){
        return NULL;
    }
    else{
        int *fd_of_deleted_client = head->client_fd;
        node_t *node_to_delete = head;
        node_t *prev=head;
        int index=rand()%size_of_queue;
        for (int i = 0; i < index; i++) {
            prev=node_to_delete;
            node_to_delete=node_to_delete->next;
            fd_of_deleted_client=node_to_delete->client_fd;
        }
        if(node_to_delete==head){
            head=head->next;
            free (node_to_delete);
        }
        else if(node_to_delete==tail){
            tail=prev;
            free (node_to_delete);

        }
        else{
            prev->next=node_to_delete->next;
            free(node_to_delete);
        }

        size_of_queue--;
        
        if(size_of_queue==0){
            head=NULL;
            tail=NULL;
        }
        return fd_of_deleted_client;
      
    }
}
