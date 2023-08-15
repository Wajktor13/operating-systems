#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/times.h>


#ifndef BUFF_BLOCK_SIZE
#define BUFF_BLOCK_SIZE 1
#endif


char* read_file(char* input_file_name) {
    FILE* input_file = fopen(input_file_name, "r");
    long file_size;
    int i = 0;
    char *buff;

    if (input_file == NULL) {
        fprintf(stderr, "[reverse_lib] error: cannot read input file\n");
        return "";
    }

    fseek(input_file, 0, SEEK_END);
    file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    buff = calloc(file_size, sizeof(char));

    while (!feof(input_file))
    {
        i += fread(&buff[i], sizeof(char), BUFF_BLOCK_SIZE, input_file);
    }

    fclose(input_file);

    return buff;
}

void reverse_buff(char *buff){
    int buff_size = strlen(buff);
    char tmp;

    for (int i = 0; i < buff_size / 2; i++){
        tmp = buff[i];
        buff[i] = buff[buff_size-i-1];
        buff[buff_size-i-1] = tmp;
    }
}

void write_buff_to_file(char* buff, char* output_file_name){
    FILE* output_file = fopen(output_file_name, "w");
    int buff_size;

    if (!output_file){
        fprintf(stderr, "[reverse_lib] error: cannot write to output file\n");
        return;
    }

    buff_size = strlen(buff);
    fwrite(buff, sizeof(char), buff_size, output_file);

    fclose(output_file);
}

int main(int argc, char** argv){
    struct timespec timespec_start, timespec_end;
    char *buff;


    if (argc != 3){
        fprintf(stderr, "[reverse_lib] error: invalid number of arguments. %d were given, 2 are required.\n", argc - 1);

    } else if (true){
        
        clock_gettime(CLOCK_REALTIME, &timespec_start);

        buff = read_file(argv[1]);
        reverse_buff(buff);
        write_buff_to_file(buff, argv[2]);

        clock_gettime(CLOCK_REALTIME, &timespec_end);

        printf("\n---------------\nexecution time: %ldns\n", timespec_end.tv_nsec - timespec_start.tv_nsec);

    } else{
        fprintf(stderr, "[reverse_lib] error: invalid arguments. Try: *String* *String*\n");
        return -1;
    }

    free(buff);

    return 0;
}