#include "shared.h"

sem_t* queue_sem;
sem_t* barbers_sem;
sem_t* chairs_sem;


int open_sems(){
    queue_sem = sem_open(QUEUE_SEM_KEY, 0);
    if(queue_sem == SEM_FAILED){
        return -1;
    }

    barbers_sem = sem_open(BARBERS_SEM_KEY, 0);
    if(barbers_sem == SEM_FAILED){
        return -1;
    }

    chairs_sem = sem_open(CHAIRS_SEM_KEY, 0);
    if(chairs_sem == SEM_FAILED){
        return -1;
    }

    return 0;
}

int random_haircut(int min, int max){
    return rand() % (max - min + 1) + min;
}

int main()
{
    setbuf(stdout, NULL);
    srand(getpid() % time(NULL));
    int pid = getpid();

    ClientsQueue* queue = open_shm();
    if(queue == NULL){
        printf("[customer%d] Error while opening shared memory\n", pid);
        return -1;
    }

    if(open_sems() == -1){
        printf("[customer%d] Error while opening semaphores\n", pid);
        return -1;
    }

    if(sem_wait(queue_sem) == -1){
        printf("[customer%d] Error while waiting for queue semaphore\n", pid);
        return -1;
    }

    if (!clients_queue_full(queue)){
        int haircut = random_haircut(1, MAX_HAIRCUTS);
        printf("[customer%d] New customer with haircut %d\n", pid, haircut);
        clients_queue_push(queue, haircut);
    } else {
        printf("[customer%d] Queue is full, leaving...\n", pid);
    }

    if(sem_post(queue_sem) == -1){
        printf("[customer%d] Error while posting queue semaphore\n", pid);
        return -1;
    }
    
    return 0;
}
