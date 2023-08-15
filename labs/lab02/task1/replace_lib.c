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
    FILE* input_file = fopen(input_file_name, "r");
    FILE* output_file = fopen(output_file_name, "w");
    long file_size;
    char *buffer, *tmp;
    int result;

    if (!input_file){
        fprintf(stderr, "[replace_lib] error: cannot open input file\n");
        fclose(output_file);
        return;
    } else if (!output_file){
        fprintf(stderr, "[replace_lib] error: cannot open output file\n");
        fclose(output_file);
        return;
    }

    fseek(input_file, 0, SEEK_END);
    file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));

    result = fread(buffer, sizeof(char), file_size, input_file);

    if (!result){
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

    result = fwrite(buffer, sizeof(char), file_size, output_file);

    if (!result){
        fprintf(stderr, "[replace_lib] error: cannot write to output file\n");
    }
    
    fclose(input_file);
    fclose(output_file);
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