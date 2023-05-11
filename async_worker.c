/*  ITCS343 - Principles of Operating Systems 
    Project 1 - Async Workers : Synchronization in Multi-threaded Programs
    
    Authors:
    Doungnapat T. 6388068 
    Nubthong W.   6388115
    Chusong X.    6388177
    
    Instructor : Our beloved Aj.Nor (Thanapon N.)

*/

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "common.h"
#include "common_threads.h"

#ifdef linux
#include <semaphore.h>
#elif __APPLE__
#include "zemaphore.h"
#endif

/* Because we declare the array in the main function, which uses  stack and it is going
    to be a static memory, so we can't just have the an array of number of inputs */

#define CMAX 10 // define max number of consumers
#define PMAX 10 // define max nubmer of producers
#define BUFFER_SIZE 5 // define maximum size of buffer

int *buffer;

int use = 0;
int fill = 0;
int issue;
int buffer_count = 0; // use to track number of buffer
int drop_req = 0;

sem_t empty;
sem_t full;
pthread_mutex_t mutex; // mutex lock


int consumers = 1;
int producers = 1; // init producers
int total_req = 0; // total number of requests
int mintime = 0;   // init minimum time to process requests
int maxtime = 0;   // init maximum time to process requests
int action;        // action from the user input
int endcase = 1;

int random_process_time = 0; // total time to process requests

double start, stop, total_time = 0; // measures time of the total process
double consumer_start, consumer_end;
double producer_start, producer_end;
double waiting_time_sum = 0;

int fill_buffer = 0; 
int count = 0;

// convert from usleep in microsec to millisec
int msleep(unsigned int tms)
{
    return usleep(tms * 1000); 
}

// assuming that the producer will issue a request every 100 to 500 ms at random.
void producer_process(){
    issue = (rand() % (500 - 100 + 1)) + 100;
    msleep(issue);
    printf("producer waits for %d ms \n",issue);
}

// the consumer will consume the package in between the minimum time and maximum time from the user input
void consumer_process()
{
   random_process_time = (rand() % (maxtime - mintime + 1)) + mintime;
   msleep(random_process_time);
   printf("consumer random process time %d \n",random_process_time);
}

void do_fill(int value)
{
    buffer[fill] = value;
    fill++;
    // printf("fill in do_fill %d\n", fill);
    if (fill == BUFFER_SIZE) // means it is full
        fill = 0;           // set fill to zero
}

int do_get()
{
    int tmp = buffer[use];
    use++;
    if (use == BUFFER_SIZE)
        use = 0;
    return tmp;
}


void *producer_wait(void *arg)
{
    int i;
    for (i = 0; i < total_req; i++)
    {
        while(fill_buffer <= total_req){
        producer_process();
        Sem_wait(&empty);
        producer_start = GetTime();
        pthread_mutex_lock(&mutex);
        fill_buffer++;
        do_fill(fill_buffer);
        pthread_mutex_unlock(&mutex);
        Sem_post(&full);
        producer_end = GetTime();
        waiting_time_sum += producer_end - producer_start;
        // printf("%f\n",waiting_time_sum);
        printf("producer id : %lld producing package : %d\n", (long long int)arg, fill_buffer);
        }
        
    }

    // end case
    
    if(endcase == 1){
        endcase = 0;
        for (i = 0; i < consumers; i++)
        {   
            producer_process();
            Sem_wait(&empty);
            pthread_mutex_lock(&mutex);
            do_fill(-1);
            pthread_mutex_unlock(&mutex);
            Sem_post(&full);
            // printf("-- producer id : %lld done producing package and fill -1 -- \n", (long long int)arg);
        }
    }
    return NULL;
}


void *producer_drop(void *arg)
{
    int i;
    for (i = 0; i < total_req; i++)
    {
        
        if(buffer_count <= BUFFER_SIZE){
            
            producer_process();
            Sem_wait(&empty);
            producer_start = GetTime();
            pthread_mutex_lock(&mutex);
            fill_buffer++;
            do_fill(fill_buffer);
            buffer_count++;
            pthread_mutex_unlock(&mutex);
            Sem_post(&full);
            producer_end = GetTime();
           
            printf("%d\n", buffer_count);
            printf("producer id : %lld producing package : %d\n", (long long int)arg, fill_buffer);
        }
        else{
            // pthread_mutex_lock(&mutex);
            producer_start = GetTime();
            drop_req++;
            producer_end = GetTime();
            printf("producer id: %lld dropped requests : %d\n", (long long int)arg , fill_buffer+1);
            // pthread_mutex_unlock(&mutex);        
        }
        // Sem_post(&full);
        // printf("producer id : %lld producing package no : %d\n", (long long int)arg ,i);
    }
    // end case
    
    if(endcase == 1){
        endcase = 0;
        for (i = 0; i < consumers; i++)
        {   
            producer_process();
            Sem_wait(&empty);
            pthread_mutex_lock(&mutex);
            do_fill(-1);
            pthread_mutex_unlock(&mutex);
            Sem_post(&full);
            // printf("-- producer id : %lld done producing package and fill -1 -- \n", (long long int)arg);
        }
    }

    return NULL;
}


void *producer_replace(void *arg)
{
    int i;
    for (i = 0; i < total_req; i++)
    {
        if(buffer_count <= BUFFER_SIZE){
            producer_process();
            Sem_wait(&empty);
            producer_start = GetTime();
            pthread_mutex_lock(&mutex);
            do_fill(i);
            buffer_count++;
            printf("buffer count = %d\n", buffer_count);
            pthread_mutex_unlock(&mutex);
            Sem_post(&full);
            producer_end = GetTime();
        }
        else{

            pthread_mutex_lock(&mutex);
            printf("oldest buffer = %d\n",buffer[use]);
            buffer[use] = i;
            use++;
            printf("replace with %d\n", buffer[use]);
            if (use == BUFFER_SIZE){
                use = 0;
            }
            pthread_mutex_unlock(&mutex);
        }
        printf("producer id : %lld \n", (long long int)arg);
    }

    // end case
    for (i = 0; i < consumers; i++)
    {
        producer_process();
        Sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        do_fill(-1);
        pthread_mutex_unlock(&mutex);
        Sem_post(&full);
        producer_process();
        printf("-- producer id : %lld done producing package and fill -1 -- \n", (long long int)arg);
    }

    return NULL;
}


void *consumer(void *arg)
{
    int tmp = 0;
    while (tmp != -1)
    {  
        consumer_process();
        consumer_start = GetTime();
        Sem_wait(&full);
        pthread_mutex_lock(&mutex);
        tmp = do_get();
        // buffer_count--;
        pthread_mutex_unlock(&mutex);
        Sem_post(&empty);
        consumer_end = GetTime();
        printf("consumer id: %lld ate the package no. %d\n", (long long int)arg, tmp);
        // float total = consumer_end - consumer_start;
        // printf("consumer processing for %lf ms \n",total * 1000);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        fprintf(stderr, "usage: %s <producer> <consumers> <total_req> <mintime> <maxtime> <action> \n", argv[0]);
        exit(1);
    }
    producers = atoi(argv[1]); // number of producers
    consumers = atoi(argv[2]); // number of consumers
    total_req = atoi(argv[3]); // total number of requests
    mintime = atoi(argv[4]);   // min time to process is ms
    maxtime = atoi(argv[5]);   // max time to process is ms
    action = atoi(argv[6]);    // 1:wait vs 2:drop vs 3:replace

    buffer = (int *)malloc(BUFFER_SIZE * sizeof(int));

    for (int i = 0; i <= BUFFER_SIZE; i++)
    {
        buffer[i] = -42;
    }

    Sem_init(&empty, BUFFER_SIZE);    // max are empty
    Sem_init(&full, 0);               // 0 are full
    pthread_mutex_init(&mutex, NULL); // initialize mutex

    pthread_t pid[PMAX], cid[CMAX];
    int i;

    start = GetTime(); // start the timer

    printf("Simulation started..\n");

    if (action == 1) {
        for (i = 0; i < producers; i++){
            Pthread_create(&pid[i], NULL, producer_wait, (void *)(long long int)i);
        }
    }
    else if (action == 2) {
        for (i = 0; i < producers; i++){
            Pthread_create(&pid[i], NULL, producer_drop, (void *)(long long int)i);
        }
    }
    else if (action == 3){
        for (i = 0; i < producers; i++){
            Pthread_create(&pid[i], NULL, producer_replace, (void *)(long long int)i);
        }
    }


    for (i = 0; i < consumers; i++)
    {
        Pthread_create(&cid[i], NULL, consumer, (void *)(long long int)i);
    }

    // wait for all threads to finish
    for (int i = 0; i < producers; i++)
    {
        Pthread_join(pid[i], NULL);
    }

    for (i = 0; i < consumers; i++)
    {
        Pthread_join(cid[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    
    stop = GetTime();
    total_time = stop - start; // stop the timer

    printf("\n");
    printf("/-------------------------------------------------/\n\n");
    if (action == 1){
        printf("Action when the queue is full = Wait\n");
    } else if (action == 2){
        printf("Action when the queue is full = Drop\n");
    } else if (action == 3){
        printf("Action when the queue is full = Replace\n");
    }

    // printf("Waiting time is %f\n", waiting_time_sum);
    printf("Average Waiting time of the process %f\n", waiting_time_sum/total_req);
    printf("Number of dropped requests %d\n",drop_req);
    printf("Number of total requests %d\n",total_req);
    printf("Percentage of the drop requests %.2f %%\n",(float) (drop_req*100)/total_req);
    // printf("consumer start time= %f \n",consumer_start);
    // printf("consumer end time= %f \n",consumer_end);

    printf("Total time of the simulation is %.2f seconds \n\n", total_time);
    printf("/---------------------------------------------------/\n");
    printf("\n Simulation ended, so is my life..\n");
    return 0;
}
