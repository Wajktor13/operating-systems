#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define INTERVAL_START 0
#define INTERVAL_END 1
#define RESULTS_PATH "/tmp/sgc_results.txt"
#define PIPE_BUFFER_SIZE 256


double get_execution_time(struct timespec start, struct timespec end)
{
    struct timespec diff;
    
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        diff.tv_sec = end.tv_sec - start.tv_sec - 1;
        diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        diff.tv_sec = end.tv_sec - start.tv_sec;
        diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    
    return diff.tv_sec + diff.tv_nsec / 1000000000.0;
}

int main(int argc, char *argv[]){
    double rectangle_width;
    int n;
    double integral_value = 0;
    double singular_interval;
    struct timespec timespec_start, timespec_end;
    double execution_time;

    clock_gettime(CLOCK_REALTIME, &timespec_start);

    if (argc != 3){
        fprintf(stderr, "Wrong number of arguments\n");
        return 1;
    }

    rectangle_width = atof(argv[1]);

    if (rectangle_width <= 0){
        fprintf(stderr, "Wrong rectangle width\n");
        return 1;
    }

    if (rectangle_width > (INTERVAL_END - INTERVAL_START)){
        fprintf(stderr, "Rectangle width is too big\n");
        return 1;
    }

    n = atoi(argv[2]);

    if (n <= 0){
        fprintf(stderr, "Wrong number of child programs\n");
        return 1;
    }

    singular_interval = ((double)INTERVAL_END - (double)INTERVAL_START) / n;

    if (singular_interval < rectangle_width){
        fprintf(stderr, "Rectangle width is too big for this number of programs\n");
        return 1;
    }
    
    mkfifo(RESULTS_PATH, 0666);

    char arg_i[PIPE_BUFFER_SIZE];

    for (int i = 0; i < n; i++){
        if (!fork()){
            snprintf(arg_i, PIPE_BUFFER_SIZE, "%d", i);
            execl("./sgc", "sgc", argv[1], argv[2], arg_i, NULL);
            exit(0);
        }
    }

    int sgc_results = open(RESULTS_PATH, O_RDONLY);
    int size_read;
    char *read_buffer = malloc(PIPE_BUFFER_SIZE * sizeof(char));

    for (int i = 0; i < n; i++) {
        size_read = read(sgc_results, read_buffer, PIPE_BUFFER_SIZE);
        read_buffer[size_read] = 0;
        
        char d[]="\n";
        char *token = strtok(read_buffer, d);

        for(;token; token=strtok(NULL, d)){
            integral_value += atof(token);
        }
    }

    printf("Result: %.16lf\n", integral_value);

    clock_gettime(CLOCK_REALTIME, &timespec_end);
    execution_time = get_execution_time(timespec_start, timespec_end);
    printf("Execution time: %f\n", execution_time);

    close(sgc_results);
    free(read_buffer);

    return 0;
}