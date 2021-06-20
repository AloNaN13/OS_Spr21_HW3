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

    // thread_stats_t* pool_threads_stats = (thread_stats_t*)malloc(size_thread_pool * sizeof(thread_stats_t)); // looks like we don't need it here

    for (int i = 0; i < size_thread_pool; i++) {
        thread_stats_t* work_thread_stats = (thread_stats_t*)malloc(sizeof(thread_stats_t));
        work_thread_stats->thread_id = i;
        /*int* args_for_thread_func = (int*)malloc(sizeof(int)*2);//the parameters for function_for_thread_in_pool
        args_for_thread_func[1]=(max_size_of_queue);
        args_for_thread_func[0]=i; //the special id of the thread */
        pthread_create(&pool_threads[i], NULL, function_for_thread_in_pool,work_thread_stats);
        //free(args_for_thread_func);
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
            //overload handle
            if(handle_for_overload(connfd,alg_to_handle_overload,arrival_time)){//if its
                continue;
            }
        }
        int *pclient=malloc(sizeof(int));
        *pclient=connfd;
        pthread_mutex_lock(&mutex_for_queue);   //lock so we can enqueue
        enqueue(pclient, arrival_time);
        pthread_mutex_unlock(&mutex_for_queue); //release lock
        pthread_cond_signal(&condition_var);   //send signal that a new request was added to the queue

    }

}

void *function_for_thread_in_pool(thread_stats_t* work_thread_stats){

    //size?
    //thread_stats_t* work_thread_stats = (thread_stats_t*)malloc(sizeof(thread_stats_t));
    //work_thread_stats->thread_id = *((int*)(args));
    
    work_thread_stats->count_req = 0;
    work_thread_stats->static_req = 0;
    work_thread_stats->dynamic_req = 0;

    while (1){
        int* pclient;
        struct timeval * arrival_time=(struct timeval*)malloc(sizeof(struct timeval));
        pthread_mutex_lock(&mutex_for_queue); //lock
        if ((pclient = dequeue(arrival_time)) == NULL){
            //release lock and put thread to sleep until we add a new request to the queue
            pthread_cond_wait(&condition_var, &mutex_for_queue);
            pclient = dequeue(arrival_time); //pop a request to handle
        }
        struct timeval* ending_time=(struct timeval*)malloc(sizeof(struct timeval));
        gettimeofday(ending_time,NULL);
        struct timeval * dispatch_time=(struct timeval*)malloc(sizeof(struct timeval));
        timersub (ending_time,arrival_time, dispatch_time);

        cur_working_threads++;


        pthread_mutex_unlock(&mutex_for_queue); //release lock
        if (pclient != NULL) {
            //HANDLE CONNECTION
            //pthread_mutex_lock(&mutex_for_curr_workers_num);
            //cur_working_threads++;
            //pthread_mutex_unlock(&mutex_for_curr_workers_num);
            requestHandle((*(int*)pclient), work_thread_stats, *arrival_time, *dispatch_time);

            pthread_mutex_lock(&mutex_for_queue);
            cur_working_threads--;
            pthread_mutex_unlock(&mutex_for_queue);



            //pthread_mutex_lock(&mutex_for_curr_workers_num);
            //cur_working_threads--;
            //pthread_mutex_unlock(&mutex_for_curr_workers_num);

            pthread_cond_signal(&condition_var_for_full_queue);
            
            Close(*pclient);
            free(pclient);
            
        }
    }

}
int handle_for_overload(int connfd, char *alg_to_handle_overload,struct timeval* arrival_time){
    if(!strcmp(alg_to_handle_overload,"block")){
        //Close(connfd);
        pthread_mutex_lock(&mutex_for_curr_workers_num);
        pthread_cond_wait(&condition_var_for_full_queue, &mutex_for_curr_workers_num);//wait for queue to stop being full
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
        printf("size of queue is :%d and mum_deques is : %d , current woriking is : %d\n",size_of_queue, num_dequeus, cur_working_threads);
        for(int j=0;j<num_dequeus;j++){
             fd_to_close=rand_dequeue();
             close(*fd_to_close);
             free(fd_to_close);
        }
        pthread_mutex_unlock(&mutex_for_queue);
        return 0;
        
    }
    

    printf("ERROR_UNDEFINED_ALGO_EXPERT");
    return 0;

}

    


 
