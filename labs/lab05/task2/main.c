#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/times.h>

#define INTERVAL_START 0
#define INTERVAL_END 1
#define PIPE_BUFF_SIZE 1024


double f(double x){
    return 4 / (x * x + 1);
}

double rectangle_area(int i, double width, double singular_interval){
    double area = 0;
    double x = INTERVAL_START + i * singular_interval;

    while (x < INTERVAL_START + (i + 1) * singular_interval){
        area += f(x) * width;
        x += width;
    }

    return area;
}


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
    int *pipe_outs;
    int to_write_size;
    char *buff;
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
        fprintf(stderr, "Wrong number of child processes\n");
        return 1;
    }

    singular_interval = ((double)INTERVAL_END - (double)INTERVAL_START) / n;

    if (singular_interval < rectangle_width){
        fprintf(stderr, "Rectangle width is too big for this number of processes\n");
        return 1;
    }

    buff = malloc(PIPE_BUFF_SIZE * sizeof(char));
    pipe_outs = malloc(n * sizeof(int));


    for (int i = 0; i < n; i++){
        int fd[2];
        if (pipe(fd) == -1){
            fprintf(stderr, "Error while creating pipe\n");
            free(buff);
            free(pipe_outs);
            return 1;
        }

        if (fork()){
            close(fd[1]);
            pipe_outs[i] = fd[0];
        } else {
            close(fd[0]);
            double area = rectangle_area(i, rectangle_width, singular_interval);
            to_write_size = snprintf(buff, PIPE_BUFF_SIZE, "%lf", area);

            if (write(fd[1], buff, to_write_size) == -1){
                fprintf(stderr, "Error while writing to pipe\n");
                free(buff);
                free(pipe_outs);
                return 1;
            }
            
            free(buff);
            free(pipe_outs);

            exit(0);
        }
    }

    while (wait(NULL) > 0);

    for (int i = 0; i < n; i++) {
        if(read(pipe_outs[i], buff, PIPE_BUFF_SIZE) == -1){
            fprintf(stderr, "Error while reading from pipe\n");
            free(buff);
            free(pipe_outs);
            return 1;
        }

        integral_value += strtod(buff, NULL);
    }

    printf("Result: %.16lf\n", integral_value);

    clock_gettime(CLOCK_REALTIME, &timespec_end);
    execution_time = get_execution_time(timespec_start, timespec_end);
    printf("Execution time: %f\n", execution_time);

    free(pipe_outs);
    free(buff);

    return 0;
}