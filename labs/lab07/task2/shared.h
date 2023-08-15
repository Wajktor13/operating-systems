#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>

#define SHM_KEY "/shm"
#define SHM_SIZE 1024
#define QUEUE_SEM_KEY "qkjebfispwas"
#define BARBERS_SEM_KEY "tuoiwizsmdwx"
#define CHAIRS_SEM_KEY "wriuoqwnxqsnl"

#define QUEUE_SIZE 8
#define MAX_BARBERS 4
#define MAX_CHAIRS 3
#define MAX_CUSTOMERS 15
#define MAX_HAIRCUTS 6


typedef struct ClientsQueue{
    int haircuts[QUEUE_SIZE];
    int size;
}ClientsQueue;


ClientsQueue* open_shm(){
    int desc = shm_open(SHM_KEY, O_RDWR | O_CREAT, 0644);
    if(desc == -1){
        return NULL;
    }

    if(ftruncate(desc, SHM_SIZE) == -1){
        return NULL;
    }

    ClientsQueue* queue = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, desc, 0);
    if(queue == (void*)-1){
        return NULL;
    }

    return queue;
}

int close_shm(){
    if(shm_unlink(SHM_KEY) == -1){
        return -1;
    }

    return 0;
}

int unlink_sems(){
    if(sem_unlink(QUEUE_SEM_KEY) == -1){
        return -1;
    }

    if(sem_unlink(BARBERS_SEM_KEY) == -1){
        return -1;
    }

    if(sem_unlink(CHAIRS_SEM_KEY) == -1){
        return -1;
    }

    return 0;
}

sem_t* open_sem(char *filepath){
    sem_t *sem_id = sem_open(filepath, 0);

    if(sem_id == SEM_FAILED){
        return NULL;
    }

    return sem_id;
}

bool clients_queue_empty(struct ClientsQueue* queue){
    return queue->size == 0;
}

bool clients_queue_full(struct ClientsQueue* queue){
    return queue->size == QUEUE_SIZE;
}

int clients_queue_push(struct ClientsQueue* queue, int client){
    if(clients_queue_full(queue)){
        return -1;
    }

    queue->haircuts[queue->size] = client;
    queue->size++;

    return 0;
}

int clients_queue_pop(struct ClientsQueue* queue){
    if(clients_queue_empty(queue)){
        printf("Error: clients_queue_pop() failed - queue is empty\n");
        exit(1);
    }

    int client = queue->haircuts[0];
    for(int i = 0; i < queue->size - 1; i++){
        queue->haircuts[i] = queue->haircuts[i + 1];
    }
    queue->size--;

    return client;
}

#endif