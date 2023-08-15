#include "shared.h"

#define BARBER_EXE "./barber"
#define CUSTOMER_EXE "./customer"

sem_t* queue_sem;
sem_t* barbers_sem;
sem_t* chairs_sem;

sem_t* create_single_sem(char *filepath, int init_val){
    sem_t *sem_id = sem_open(filepath, O_CREAT | O_EXCL, 0644, init_val);

    if(sem_id == SEM_FAILED){
        perror("[main] Error: sem_open() failed");
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
    if(sem_close(queue_sem) == -1){
        perror("[main] Error: sem_close() failed");
        exit(1);
    }

    if(sem_close(barbers_sem) == -1){
        perror("[main] Error: sem_close() failed");
        exit(1);
    }

    if(sem_close(chairs_sem) == -1){
        perror("[main] Error: sem_close() failed");
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
    
    if(unlink_sems() == -1){
        perror("[main] Error: close_sems() failed");
        exit(1);
    }

    if(close_shm() == -1){
        perror("[main] Error: close_shm() failed");
        exit(1);
    }

    return 0;
}