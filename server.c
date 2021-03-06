#include "segel.h"
#include "request.h"

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

void *function_for_thread_in_pool(thread_stats_t* work_thread_stats);
int handle_for_overload(int connfd, char *alg_to_handle_overload,struct timeval* arrival_time);
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


    for (int i = 0; i < size_thread_pool; i++) {
        thread_stats_t* work_thread_stats = (thread_stats_t*)malloc(sizeof(thread_stats_t));
        work_thread_stats->thread_id = i;
       
        pthread_create(&pool_threads[i], NULL, function_for_thread_in_pool,work_thread_stats);
    }


    listenfd = Open_listenfd(port);
    while (1) {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

	    // assign arrival time
        struct timeval* arrival_time=(struct timeval*)malloc(sizeof(struct timeval));
        gettimeofday(arrival_time,NULL);
         


	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	//

	/*
	    requestHandle(connfd);

	    Close(connfd);*/


        if(size_of_queue + cur_working_threads >= max_size_of_queue){
            
            if(handle_for_overload(connfd,alg_to_handle_overload,arrival_time)){//if we have to many clients in the system
                continue;
            }
        }
        int *pclient=malloc(sizeof(int));
        *pclient=connfd;
        pthread_mutex_lock(&mutex_for_queue);  
        enqueue(pclient, arrival_time);
        pthread_mutex_unlock(&mutex_for_queue); 
        pthread_cond_signal(&condition_var);   

    }

}

void *function_for_thread_in_pool(thread_stats_t* work_thread_stats){

    //size?
    //thread_stats_t* work_thread_stats = (thread_stats_t*)malloc(sizeof(thread_stats_t));
    //work_thread_stats->thread_id = *((int*)(args));
    
    work_thread_stats->count_req = 0;
    work_thread_stats->static_req = 0;
    work_thread_stats->dynamic_req = 0;


    struct timeval* arrival_time=(struct timeval*)malloc(sizeof(struct timeval));
    struct timeval* ending_time=(struct timeval*)malloc(sizeof(struct timeval));
    struct timeval* dispatch_time=(struct timeval*)malloc(sizeof(struct timeval));

    while (1){
        int* pclient;
        pthread_mutex_lock(&mutex_for_queue); //lock because we will change the queue
        if ((pclient = dequeue(arrival_time)) == NULL){
           
            pthread_cond_wait(&condition_var, &mutex_for_queue);  
            pclient = dequeue(arrival_time); 
        }
        
        gettimeofday(ending_time,NULL);
        timersub (ending_time,arrival_time, dispatch_time);

        cur_working_threads++;


        pthread_mutex_unlock(&mutex_for_queue); //unlock
        if (pclient != NULL) {
            //HANDLE CONNECTION
           
            requestHandle((*(int*)pclient), work_thread_stats, *arrival_time, *dispatch_time);

            pthread_mutex_lock(&mutex_for_queue);
            cur_working_threads--;
            pthread_mutex_unlock(&mutex_for_queue);



            pthread_cond_signal(&condition_var_for_full_queue);
            
            Close(*pclient);
            free(pclient);
            
        }
    }

}
int handle_for_overload(int connfd, char *alg_to_handle_overload,struct timeval* arrival_time){
    if(!strcmp(alg_to_handle_overload,"block")){
        pthread_mutex_lock(&mutex_for_curr_workers_num);
        pthread_cond_wait(&condition_var_for_full_queue, &mutex_for_curr_workers_num);//wait for a signal that the queue is not full
        pthread_mutex_unlock(&mutex_for_curr_workers_num);
        
        return 0;
    } else if(!strcmp(alg_to_handle_overload,"dt")){
        Close(connfd);
        return 1;
    }else if(!strcmp(alg_to_handle_overload,"dh")){
        pthread_mutex_lock(&mutex_for_queue);
        int* fd_to_close=dequeue(arrival_time);
        /*if(fd_to_close!=NULL){
            close(*fd_to_close);
            free(fd_to_close);
        }*/
        close(*fd_to_close);
        free(fd_to_close);
        pthread_mutex_unlock(&mutex_for_queue);
        return 0;
    }
   
    else if (!strcmp(alg_to_handle_overload,"rd")){
        pthread_mutex_lock(&mutex_for_queue);
        int* fd_to_close;
        int num_dequeus= size_of_queue/4;
        for(int j=0;j<num_dequeus;j++){
             fd_to_close=rand_dequeue();
             close(*fd_to_close);
             free(fd_to_close);
        }
        pthread_mutex_unlock(&mutex_for_queue);
        return 0;
        
    }
    

    printf("UNDEFINED_ALGO_OF_OVERLOAD\n");
    return 0;

}

    


 
