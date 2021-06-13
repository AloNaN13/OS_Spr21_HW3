#include "segel.h"
#include "request.h"
#include "myQueue.h"
// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too

void *function_for_thread_in_pool(void* args);
int handle_for_overload(int connfd, char *alg_to_handle_overload);
int cur_working_threads = 0;
pthread_mutex_t mutex_for_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_for_curr_workers_num = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var_for_full_queue;

void getargs(int *port, int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
}


int main(int argc, char *argv[])
{
    size_of_queue=0;
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    getargs(&port, argc, argv);

    // 
    // HW3: Create some threads...
    //



    //create pool thread
    int size_thread_pool = atoi(argv[2]);//the size of the thread pool
    int max_size_of_queue = atoi(argv[3]);
    char* alg_to_handle_overload = (argv[4]); //the algorithm that handel full queue
    pthread_t pool_threads[size_thread_pool];

    //stats_t* worker_thread_stats=(stats_t*)malloc(sizeof(stats_t)*num_of_workers);

    for (int i = 0; i < size_thread_pool; i++) {
        int* args_for_thread_func = (int*)malloc(sizeof(int)*2);//the parameters for function_for_thread_in_pool
        args_for_thread_func[0]=(max_size_of_queue);
        args_for_thread_func[1]=(i);//the special id of the thread
        pthread_create(&pool_threads[i], NULL, function_for_thread_in_pool,(void*)args_for_thread_func);
        free(args_for_thread_func);
    }


    listenfd = Open_listenfd(port);
    while (1) {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	//

	/*
	    requestHandle(connfd);

	    Close(connfd);*/
        if(size_of_queue + cur_working_threads >= max_size_of_queue){
            //overload handle
            if(handle_for_overload(connfd,alg_to_handle_overload)){//if its
                continue;
            }
        }
        pthread_mutex_lock(&mutex_for_queue);   //lock so we can enqueue
        enqueue((void *)(&connfd));
        pthread_mutex_unlock(&mutex_for_queue); //realse lock
        pthread_cond_signal(&condition_var);   //send signal that a new request was added to the queue

    }

}

void *function_for_thread_in_pool(void* args){

    while (1){
        void* pclient;
        pthread_mutex_lock(&mutex_for_queue); //lock
        if ((pclient = dequeue()) == NULL){
            //relase lock and put thread to sleep until we add a new request to the queue
            pthread_cond_wait(&condition_var, &mutex_for_queue);
            pclient = dequeue(); //pop a request to handle
        }
        cur_working_threads++;


        pthread_mutex_unlock(&mutex_for_queue); //relase lock
        if (pclient != NULL) {
            //HANDLE CONNECTION
            //pthread_mutex_lock(&mutex_for_curr_workers_num);
            //cur_working_threads++;
            //pthread_mutex_unlock(&mutex_for_curr_workers_num);
            requestHandle((*(int*)pclient));

            pthread_mutex_lock(&mutex_for_queue);
            cur_working_threads--;
            pthread_mutex_unlock(&mutex_for_queue);



            //pthread_mutex_lock(&mutex_for_curr_workers_num);
            //cur_working_threads--;
            //pthread_mutex_unlock(&mutex_for_curr_workers_num);

            pthread_cond_signal(&condition_var_for_full_queue);
            printf("file d is:%d", *((int*)pclient));
            Close(*((int*)pclient));
            
        }
    }

}
int handle_for_overload(int connfd, char *alg_to_handle_overload){
    if(!strcmp(alg_to_handle_overload,"block")){
        Close(connfd);
        pthread_mutex_lock(&mutex_for_curr_workers_num);
        pthread_cond_wait(&condition_var_for_full_queue, &mutex_for_curr_workers_num);//wait for queue to stop being full
        pthread_mutex_unlock(&mutex_for_curr_workers_num);
        return 1;
    } else if(!strcmp(alg_to_handle_overload,"dt")){
        Close(connfd);
        return 1;
    }else if(!strcmp(alg_to_handle_overload,"dh")){
        dequeue();
        return 0;
    }
    ///bonus



    printf("ERROR_UNDEFINED_ALGO_EXPERT");
    return 0;



}

    


 
