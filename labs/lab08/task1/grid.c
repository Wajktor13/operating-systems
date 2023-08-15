#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include "grid.h"


const int grid_width = 30;
const int grid_height = 30;

char *create_grid()
{
    return malloc(sizeof(char) * grid_width * grid_height);
}

void destroy_grid(char *grid)
{
    free(grid);
}

void draw_grid(char *grid)
{
    for (int i = 0; i < grid_height; ++i)
    {
        // Two characters for more uniform spaces (vertical vs horizontal)
        for (int j = 0; j < grid_width; ++j)
        {
            if (grid[i * grid_width + j])
            {
                mvprintw(i, j * 2, "■");
                mvprintw(i, j * 2 + 1, " ");
            }
            else
            {
                mvprintw(i, j * 2, " ");
                mvprintw(i, j * 2 + 1, " ");
            }
        }
    }

    refresh();
}

void init_grid(char *grid)
{
    for (int i = 0; i < grid_width * grid_height; ++i)
        grid[i] = rand() % 2 == 0;
}

bool is_alive(int row, int col, char *grid)
{

    int count = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0)
            {
                continue;
            }
            int r = row + i;
            int c = col + j;
            if (r < 0 || r >= grid_height || c < 0 || c >= grid_width)
            {
                continue;
            }
            if (grid[grid_width * r + c])
            {
                count++;
            }
        }
    }

    if (grid[row * grid_width + col])
    {
        if (count == 2 || count == 3)
            return true;
        else
            return false;
    }
    else
    {
        if (count == 3)
            return true;
        else
            return false;
    }
}

void update_grid(char *src, char *dst)
{
    for (int i = 0; i < grid_height; ++i)
    {
        for (int j = 0; j < grid_width; ++j)
        {
            dst[i * grid_width + j] = is_alive(i, j, src);
        }
    }
}

/////////////////////////////////////////////////////////////

typedef struct {
    char* src;
    char* dst;
    int i;
} ThreadArgs;

void ignore_signal(int signo, siginfo_t* info, void* context) {}

void* update_cell(void* void_args){
    ThreadArgs* args = (ThreadArgs*) void_args;
    char* src = args->src;
    char* dst = args->dst;
    int i = args->i;

    int row = i / grid_width;
    int col = i % grid_width;

    while (true)
    {
        dst[i] = is_alive(row, col, src);
        pause();

        char* tmp = src;
        src = dst;
        dst = tmp;
    }

    return NULL;
}

void create_threads_fnc(char *src, char *dst, int n, pthread_t *threads){
    for (int i = 0; i < n; i++){
        ThreadArgs *args = (ThreadArgs*) malloc(sizeof(ThreadArgs));
        args->src = src;
        args->dst = dst;
        args->i = i;

        pthread_create(&threads[i], NULL, update_cell, (void*) args);
    }
}

void threaded_update_grid(char *src, char *dst, bool create_threads){
    int n = grid_height * grid_width;
    static pthread_t *threads;

    struct sigaction act;
    act.sa_sigaction = ignore_signal;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);

    if (create_threads){

        threads = malloc(sizeof(pthread_t) * n);

        create_threads_fnc(src, dst, n, threads);
    }

    for (int i=0; i < n; i++) pthread_kill(threads[i], SIGUSR1);
}
