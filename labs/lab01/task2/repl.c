#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <regex.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>

#include "dll_lib_loader.h"
#include "libbm.h"

#define COMMAND_SIZE 1025


typedef struct
{
    regex_t init;
    regex_t count;
    regex_t show;
    regex_t delete;
    regex_t destroy;
    regex_t exit;
} CommandPatterns;


void execute_init(libbmStorage* storage, int size, bool* storage_created_ptr){
    if (!(*storage_created_ptr)){
        if (size == 0){
            fprintf(stderr, "error: cannot create storage of size: 0\n");  
        } else{
            *storage_created_ptr = true;
            *storage = libbm_create_storage(size);
            printf("created storage with size: %d\n", size);
        }
    } else{
        fprintf(stderr, "error: storage is already created\n");
    }
}

void execute_count(libbmStorage* storage, char* file_name, bool* storage_created_ptr){
    int ind;

    if (*storage_created_ptr){
        ind = libbm_wc_write_block(storage, file_name);
        if (ind != -1){
            printf("output has been written to block at index: %d\n", ind);
        }
    } else{
        fprintf(stderr, "error: cannot use 'count' without storage. Use 'init' first\n");
    }
}

void execute_show(libbmStorage* storage, int ind, bool* storage_created_ptr){
    if (*storage_created_ptr){
        puts(libbm_get_block(storage, ind));
    } else{
        fprintf(stderr, "error: cannot use 'show' without storage. Use 'init' first\n");
    }
}

void execute_delete(libbmStorage* storage, int ind, bool* storage_created_ptr){
    if (*storage_created_ptr){
        if(libbm_remove_block(storage, ind)){
            printf("deleted block at index: %d\n", ind);
        }
    } else{
        fprintf(stderr, "error: cannot use 'delete' without storage. Use 'init' first\n");
    }
}

void execute_destroy(libbmStorage* storage, bool* storage_created_ptr){
    if (*storage_created_ptr){
        libbm_free_blocks(storage);
        *storage_created_ptr = false;
        printf("storage has been destroyed\n");
    }
}

void execute_exit(libbmStorage* storage, bool* storage_created_ptr, bool* repl_running_ptr){
    execute_destroy(storage, storage_created_ptr);
    *repl_running_ptr = false;
}

void compile_regex_patterns(CommandPatterns* command_patterns) {

    int status = regcomp(&(command_patterns->init), "init [0-9]+", REG_EXTENDED) + regcomp(&(command_patterns->count), "count .+", REG_EXTENDED) +
    regcomp(&(command_patterns)->show, "show [0-9]+", REG_EXTENDED) + regcomp(&(command_patterns)->delete, "delete [0-9]+", REG_EXTENDED) + 
    regcomp(&(command_patterns->destroy), "destroy", REG_EXTENDED) + regcomp(&(command_patterns->exit), "exit", REG_EXTENDED);

    if (status != 0){
        fprintf(stderr, "error: failed to compile regex\n");
    }
}

void match_regex_and_execute(char* command, libbmStorage* storage,bool* repl_running_ptr, bool* storage_created_ptr){
    int num_input;
    char* str_input = malloc(COMMAND_SIZE * sizeof(char));
    int str_input_length;
    CommandPatterns command_patterns;

    compile_regex_patterns(&command_patterns);

 if (regexec(&(command_patterns.init), command, 0, NULL, 0) == 0) {
        sscanf(command, "init %d", &num_input);
        execute_init(storage, num_input, storage_created_ptr);

    } else if (regexec(&(command_patterns.count), command, 0, NULL, 0) == 0) {
        command += strlen("count ");
        str_input_length = strcspn(command, "\n");
        strncpy(str_input, command, str_input_length < COMMAND_SIZE ? str_input_length : COMMAND_SIZE);
        execute_count(storage, str_input, storage_created_ptr);

    } else if (regexec(&(command_patterns.show), command, 0, NULL, 0) == 0) {
        sscanf(command, "show %d", &num_input);
        execute_show(storage, num_input, storage_created_ptr);
        
    } else if (regexec(&(command_patterns.delete), command, 0, NULL, 0) == 0) {
        sscanf(command, "delete %d\n", &num_input);
        execute_delete(storage, num_input, storage_created_ptr);

    } else if (regexec(&(command_patterns.destroy), command, 0, NULL, 0) == 0) {
        execute_destroy(storage, storage_created_ptr);

    } else if (regexec(&(command_patterns.exit), command, 0, NULL, 0) == 0) {
        execute_exit(storage, storage_created_ptr, repl_running_ptr);

    } else {
        printf("error: invalid syntax. Try: \n'init *int*'\n'count *string*'\n'show *int*'\n'delete *int*'\n'destroy'\n'exit'\n");
    }

    free(str_input);
}

int main(){
    load_lib("./libbm.so");

    char command[COMMAND_SIZE];
    bool repl_running = true;
    bool storage_created = false;
    libbmStorage storage;
    struct timespec timespec_start, timespec_end;
    struct tms tms_start, tms_end;

    while(repl_running) {
        printf("\nREPL >>> ");
        if(fgets(command, COMMAND_SIZE, stdin) == NULL) {
            fprintf(stderr, "error: cannot read stdin\n");
            continue;
        }

        clock_gettime(CLOCK_REALTIME, &timespec_start);
        times(&tms_start);

        match_regex_and_execute(command, &storage, &repl_running, &storage_created);

        clock_gettime(CLOCK_REALTIME, &timespec_end);
        times(&tms_end);

        printf("---------------\nexecution time:\n");
        printf("real: %ld ns\n", timespec_end.tv_nsec - timespec_start.tv_nsec);
        printf("user: %ld\n", tms_end.tms_cutime - tms_start.tms_cutime);
        printf("system: %ld\n", tms_end.tms_cstime - tms_start.tms_cstime);
    }

    return 0;
}
