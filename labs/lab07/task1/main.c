#include "shared.h"

#define BARBER_EXE "./barber"
#define CUSTOMER_EXE "./customer"

int queue_sem;
int barbers_sem;
int chairs_sem;


int create_single_sem(long num, int init_val){
    key_t key = num;
    if(key == -1){
        perror("[main] Error: ftok() failed");
        exit(1);
    }

    int sem_id = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if(sem_id == -1){
        perror("[main] Error: semget() failed");
        exit(1);
    }

    union semun arg;
    arg.val = init_val;
    if(semctl(sem_id, 0, SETVAL, arg) == -1){
        perror("[main] Error: semctl() failed");
        exit(1);
    }

    return sem_id;
}

void create_sems(){
    queue_sem = create_single_sem(QUEUE_SEM_KEY, 1);
    barbers_sem = create_single_sem(BARBERS_SEM_KEY, MAX_BARBERS);
    chairs_sem = create_single_sem(CHAIRS_SEM_KEY, MAX_CHAIRS);
}

void close_sems(){
    if(semctl(queue_sem, 0, IPC_RMID) == -1){
        perror("[main] Error: semctl() failed");
        exit(1);
    }

    if(semctl(barbers_sem, 0, IPC_RMID) == -1){
        perror("[main] Error: semctl() failed");
        exit(1);
    }

    if(semctl(chairs_sem, 0, IPC_RMID) == -1){
        perror("[main] Error: semctl() failed");
        exit(1);
    }
}   

void close_sems_and_exit(){
    close_sems();
    exit(0);
}

int main(){
    signal(SIGINT, close_sems_and_exit);
    setbuf(stdout, NULL);

    printf("[main] Config:\n-queue size: %d\n-barbers: %d\n-chairs: %d\n-customers: %d\n-haircuts: %d\n",
    QUEUE_SIZE, MAX_BARBERS, MAX_CHAIRS, MAX_CUSTOMERS, MAX_HAIRCUTS);

    ClientsQueue *q = open_shm();
    if (q == NULL){
        perror("[main] Error: open_shm() failed");
        exit(1);
    }

    q->size=0;

    create_sems();

    for(int i=0; i<MAX_BARBERS; i++){
        if(fork() == 0){
            execl(BARBER_EXE, BARBER_EXE, NULL);
            printf("[main] Barber %d created\n", i);
            exit(0);
        }
    }

    printf("[main] Barbers created\n");

    for(int i=0; i<MAX_CUSTOMERS; i++){
        if(fork() == 0){
            execl(CUSTOMER_EXE, CUSTOMER_EXE, NULL);
            exit(0);
        }
    }

    printf("[main] Customers created\n");

    while (wait(NULL) > 0);

    close_sems();

    if(close_shm() == -1){
        perror("[main] Error: close_shm() failed");
        exit(1);
    }

    return 0;
}