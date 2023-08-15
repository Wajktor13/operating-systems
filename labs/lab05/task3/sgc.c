#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PIPE_BUFFER_SIZE 256
#define INTERVAL_START 0
#define INTERVAL_END 1
#define RESULTS_PATH "/tmp/sgc_results.txt"


double f(double x){
    return 4 / (x * x + 1);
}

double rectangle_area(int i, double width, double singular_interval, double interval_start){
    double area = 0;
    double x = interval_start + i * singular_interval;

    while (x < interval_start + (i + 1) * singular_interval){
        area += f(x) * width;
        x += width;
    }

    return area;
}


int main(int argc, char *argv[]){
    double rectangle_width = atof(argv[1]);
    int n = atoi(argv[2]);
    int i = atoi(argv[3]);

    double singular_interval = ((double)INTERVAL_END - (double)INTERVAL_START) / n;


    double result = rectangle_area(i, rectangle_width, singular_interval, INTERVAL_START);
    char *to_write_buff = malloc(PIPE_BUFFER_SIZE * sizeof(char));
    int to_write_buff_size = snprintf(to_write_buff, PIPE_BUFFER_SIZE, "%lf\n", result);


    int sgc_results = open(RESULTS_PATH, O_WRONLY);

    if (sgc_results == -1){
        fprintf(stderr, "Error while opening file %s", RESULTS_PATH);
        return 1;
    }

    write(sgc_results, to_write_buff, to_write_buff_size);

    close(sgc_results);
    free(to_write_buff);

    return 0;
}