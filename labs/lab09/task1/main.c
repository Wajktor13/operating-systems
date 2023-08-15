#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#define MAX_REINDEERS 9
#define MAX_ELVES 10
#define MAX_ELVES_WAITING 3
#define MAX_TOYS_DELIVERIES 3

pthread_mutex_t reindeers_mutex;
pthread_mutex_t elves_mutex;

pthread_barrier_t reindeers_barrier;
pthread_barrier_t elves_barrier;

int reindeers_waiting = 0;
int elves_waiting = 0;


void *reindeer_cycle()
{
    int sleep_time;

    while (true)
    {
        sleep_time = rand() % 6 + 5;
        pthread_mutex_lock(&reindeers_mutex);
        printf("[reindeer%ld] going on vacation for %d seconds\n", syscall(SYS_gettid), sleep_time);
        pthread_mutex_unlock(&reindeers_mutex);
        sleep(sleep_time);
        pthread_mutex_lock(&reindeers_mutex);
        reindeers_waiting++;
        printf("[reindeer%ld] came back from vacation. Currently %d reindeers are waiting\n",
               syscall(SYS_gettid), reindeers_waiting);

        if (reindeers_waiting == MAX_REINDEERS)
        {
            printf("[reindeer%ld] waking up santa\n", syscall(SYS_gettid));
        }

        pthread_mutex_unlock(&reindeers_mutex);
        pthread_barrier_wait(&reindeers_barrier);
    }

    return NULL;
}

void *elf_cycle()
{
    int sleep_time;

    while (true)
    {
        sleep_time = rand() % 3 + 2;
        pthread_mutex_lock(&elves_mutex);
        printf("[elf%ld] working for %d seconds\n", syscall(SYS_gettid), sleep_time);
        pthread_mutex_unlock(&elves_mutex);
        sleep(sleep_time);
        pthread_mutex_lock(&elves_mutex);
        printf("[elf%ld] encountered a problem\n", syscall(SYS_gettid));

        if (elves_waiting < MAX_ELVES_WAITING){
            elves_waiting++;
            printf("[elf%ld] waiting for help. Currently %d elves are waiting\n", syscall(SYS_gettid), elves_waiting);
            if (elves_waiting == MAX_ELVES_WAITING)
            {
                printf("[elf%ld] waking up santa\n", syscall(SYS_gettid));
            }
            pthread_mutex_unlock(&elves_mutex);
            pthread_barrier_wait(&elves_barrier);
        } else {
            printf("[elf%ld] santa not available, solving the problem on his own\n", syscall(SYS_gettid));
            pthread_mutex_unlock(&elves_mutex);
        }
    }

    return NULL;
}

void santa_cycle()
{
    int sleep_time;
    int deliveries = 0;

    while (deliveries < MAX_TOYS_DELIVERIES)
    {
        pthread_mutex_lock(&reindeers_mutex);
        pthread_mutex_lock(&elves_mutex);   

        if (reindeers_waiting == MAX_REINDEERS)
        {
            printf("[santa] waking up\n");
            pthread_mutex_unlock(&elves_mutex);
            reindeers_waiting = 0;
            deliveries++;
            sleep_time = rand() % 3 + 2;
            printf("[santa] delivering toys for %d seconds\n", sleep_time);
            sleep(sleep_time);
            printf("[santa] delivering toys done. Total deliveries: %d\n", deliveries);
            pthread_mutex_unlock(&reindeers_mutex);
        } else if (elves_waiting == MAX_ELVES_WAITING) {
            printf("[santa] waking up\n");
            pthread_mutex_unlock(&reindeers_mutex);
            elves_waiting = 0;
            sleep_time = rand() % 2 + 1;
            printf("[santa] helping elves for %d seconds\n", sleep_time);
            sleep(sleep_time);
            printf("[santa] helping elves done\n");
            pthread_mutex_unlock(&elves_mutex);
        } else {
            pthread_mutex_unlock(&reindeers_mutex);
            pthread_mutex_unlock(&elves_mutex);
        }
    }

    printf("[santa] all deliveries done\n");
}

void spawn_reindeers(pthread_t *reindeers, int n)
{
    for (int i = 0; i < MAX_REINDEERS; i++)
    {
        if (pthread_create(&reindeers[i], NULL, reindeer_cycle, NULL) != 0)
        {
            fprintf(stderr, "Error while creating reindeer thread %d\n", i);
            exit(1);
        }
    }
}

void spawn_elves(pthread_t *elves, int n)
{
    for (int i = 0; i < MAX_ELVES; i++)
    {
        if (pthread_create(&elves[i], NULL, elf_cycle, NULL) != 0)
        {
            fprintf(stderr, "Error while creating elf thread %d\n", i);
            exit(1);
        }
    }
}

int main()
{
    pthread_t reindeers[MAX_REINDEERS];
    pthread_t elves[MAX_ELVES];

    srand(time(NULL));

    pthread_mutex_init(&reindeers_mutex, NULL);
    pthread_mutex_init(&elves_mutex, NULL);

    pthread_barrier_init(&reindeers_barrier, NULL, MAX_REINDEERS);
    pthread_barrier_init(&elves_barrier, NULL, MAX_ELVES_WAITING);

    spawn_reindeers(reindeers, MAX_REINDEERS);
    spawn_elves(elves, MAX_ELVES);

    santa_cycle();

    return 0;
}
