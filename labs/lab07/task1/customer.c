#include "shared.h"

int queue_sem;
int barbers_sem;
int chairs_sem;


int open_sems(){
    queue_sem = semget(QUEUE_SEM_KEY, 1, 0);
    if(queue_sem == -1){
        return -1;
    }

    barbers_sem = semget(BARBERS_SEM_KEY, 1, 0);
    if(barbers_sem == -1){
        return -1;
    }

    chairs_sem = semget(CHAIRS_SEM_KEY, 1, 0);
    if(chairs_sem == -1){
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

    if(semop(queue_sem, (struct sembuf[]){{0, -1, 0}}, 1) == -1){
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

    if(semop(queue_sem, (struct sembuf[]){{0, 1, 0}}, 1) == -1){
        printf("[customer%d] Error while posting queue semaphore\n", pid);
        return -1;
    }
    
    return 0;
}
