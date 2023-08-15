#include "shared.h"

#define TIMEOUT 6

int queue_sem;
int barbers_sem;
int chairs_sem;

float seconds_slept = 0;


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


int main(){
    setbuf(stdout, NULL);
    int pid = getpid();

    ClientsQueue *q = open_shm();
    if(q == NULL){
        printf("[barber%d] Error: open_shm() failed", pid);
        exit(1);
    }

    if(open_sems() == -1){
        printf("[barber%d] Error: open_sems() failed", pid);
        exit(1);
    }
    
    while(true){
        if(semop(queue_sem, (struct sembuf[]){{0, -1, 0}}, 1) == -1){
            printf("[barber%d] Error: sem_wait() failed", pid);
            exit(1);
        }

        if(!clients_queue_empty(q)){

            if(semop(chairs_sem, (struct sembuf[]){{0, -1, 0}}, 1) == -1){
            printf("[barber%d] Error: sem_wait() failed", pid);
            exit(1);
            }

            int haircut = clients_queue_pop(q);

            if(semop(queue_sem, (struct sembuf[]){{0, 1, 0}}, 1) == -1){
                printf("[barber%d] Error: sem_post() failed", pid);
                exit(1);
            }

            printf("[barber%d] Making haircut %d\n", pid, haircut);
            sleep(haircut);
            printf("[barber%d] Haircut %d done\n", pid, haircut);
            seconds_slept = 0;

            if(semop(chairs_sem, (struct sembuf[]){{0, 1, 0}}, 1) == -1){
                printf("[barber%d] Error: sem_post() failed", pid);
                exit(1);
            }
        } else{
            printf("[barber%d] Sleeping...\n", pid);
            if(semop(queue_sem, (struct sembuf[]){{0, 1, 0}}, 1) == -1){
                printf("[barber%d] Error: sem_post() failed", pid);
                exit(1);
            }
        }

        sleep(1);
        seconds_slept += 1;

        if(seconds_slept == TIMEOUT){
            printf("[barber%d] Timeout\n", pid);
            break;
        }
    }

    return 0;
}