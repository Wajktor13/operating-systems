#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/times.h>


bool validate_arguments(char* to_replace, char* substitute, char* input_file_name, char* output_file_name){
    if (strlen(to_replace) != 1 || strlen(substitute) != 1){
        return false;
    } else{
        return true;
    }
}


void replace(char* to_replace, char* substitute, char* input_file_name, char* output_file_name){
    int input_file = open(input_file_name, O_RDONLY);
    int output_file = open(output_file_name, O_WRONLY | O_CREAT, 0644);
    long file_size;
    char *buffer, *tmp;
    int result;
    int currentPos;

    if (!input_file){
        fprintf(stderr, "[replace_lib] error: cannot open input file\n");
        close(output_file);
        return;
    } else if (!output_file){
        fprintf(stderr, "[replace_lib] error: cannot open output file\n");
        close(output_file);
        return;
    }

    currentPos = lseek(input_file, 0, SEEK_CUR);
    file_size = lseek(input_file, 0, SEEK_END);
    lseek(input_file, currentPos, SEEK_SET); 

    buffer = (char *)malloc(file_size * sizeof(char));

    result = read(input_file, buffer, file_size);

    if (result < 0){
            fprintf(stderr, "[replace_lib] error: cannot read input file\n");
    }

    tmp = buffer;

    while (*tmp)
    {
        if (*tmp == *to_replace){
            *tmp = *substitute;
        }

        tmp++;
    }

    result = write(output_file, buffer, file_size);

    if (result < 0){
        fprintf(stderr, "[replace_lib] error: cannot write to output file\n");
    }
    
    close(input_file);
    close(output_file);
    free(buffer);
}

int main(int argc, char** argv){
    struct timespec timespec_start, timespec_end;

    if (argc != 5){
        fprintf(stderr, "[replace_lib] error: invalid number of arguments. %d were given, 4 are required.\n", argc - 1);

    } else if (validate_arguments(argv[1], argv[2], argv[3], argv[4])){

        clock_gettime(CLOCK_REALTIME, &timespec_start);
        replace(argv[1], argv[2], argv[3], argv[4]);
        clock_gettime(CLOCK_REALTIME, &timespec_end);

        printf("---------------\nexecution time: %ldns\n", timespec_end.tv_nsec - timespec_start.tv_nsec);


    } else{
        fprintf(stderr, "[replace_lib] error: invalid arguments. Try: *Char* *Char* *String* *String*\n");
        return -1;
    }

    return 0;
}