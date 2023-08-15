#include "libbm.h"

#define COMMAND_SIZE 1025


//1
libbmStorage libbm_create_storage(size_t size){
    char** blocks = calloc(size, sizeof(char*));
    bool* occupied = calloc(size, sizeof(bool));

    if (blocks == NULL || occupied == NULL){
        fprintf(stderr, "[libm] error: failed to create storage\n");
    }

    return (libbmStorage){
        .blocks = blocks,
        .occupied = occupied,
        .max_size = size,
        .current_size = 0
    };
}

//helper function
char* get_tmp_file_content(char* file_name) {
    long int file_size;
    char* buffer;
    FILE* file_pointer = fopen(file_name, "r");
    int result;
    
    if (file_pointer == NULL){
        fprintf(stderr, "[libm] error: failed to open file '%s'\n", file_name);
    }

    fseek(file_pointer, 0, SEEK_END);
    file_size = ftell(file_pointer);
    fseek(file_pointer, 0, SEEK_SET);

    buffer = calloc(file_size, sizeof(char));

    if (buffer == NULL){
        fprintf(stderr, "[libm] error: failed to create buffer '%s'\n", file_name);
    }

    result = fread(buffer, sizeof(char), file_size, file_pointer);
    if (result < 0){
        return "";
    }
    fclose(file_pointer);

    return buffer;
}

//helper function
char* execute_wc(char* file_name){
    char* wc_output;
    char command[COMMAND_SIZE] = "";
    char tmp_file_name[] = "/tmp/libm_tmp_XXXXXX";
    int tmp_file = mkstemp(tmp_file_name);
    int command_result;

    if (tmp_file == 0 || tmp_file == -1){
        fprintf(stderr, "[libm] error: failed to create tmp file '%s'\n", tmp_file_name);
        return "";
    }

    snprintf(command, COMMAND_SIZE, "wc '%s' 1> '%s' 2>/dev/null", file_name, tmp_file_name);
    command_result = system(command);

    wc_output = get_tmp_file_content(tmp_file_name);

    snprintf(command, COMMAND_SIZE, "rm -f '%s' 2>/dev/null", tmp_file_name);
    command_result = system(command);

    if (command_result < 0){
        return "";
    }

    return wc_output;
}

//2
int libbm_wc_write_block(libbmStorage* storage, char* file_name){
    char* to_write = execute_wc(file_name);

    if (storage->current_size < storage->max_size) {
        storage->blocks[storage->current_size] = to_write;
        storage->occupied[storage->current_size] = true;
        (storage->current_size) += 1;

        return storage->current_size - 1;
        
    } else {
        fprintf(stderr, "[libm] error: storage is out of blocks\n");

        return -1;
    }
}

//helper function
bool validate_ind(libbmStorage* storage, int ind){
    if (storage->max_size <= ind){
        fprintf(stderr, "[libm] error: index out of range: %d\n", ind);
        return false;

    } else if (!storage->occupied[ind]){
        fprintf(stderr, "[libm] error: there is no block at index %i\n", ind);
        return false;
    } else{
        return true;
    }
}

//3
char* libbm_get_block(libbmStorage* storage, size_t ind){
    if (validate_ind(storage, ind)){
        return storage->blocks[ind];
    }
    
    return "";
}

//4
bool libbm_remove_block(libbmStorage* storage, size_t ind){
    if (validate_ind(storage, ind)){
        free(storage->blocks[ind]);
        storage->occupied[ind] = 0;

        return true;
    }

    return false;
}


//5
void libbm_free_blocks(libbmStorage* storage){
    for (int i = 0; i < storage->current_size; i++){
        if (storage->occupied[i]){
            free(storage->blocks[i]);
            storage->occupied[i] = false;
        }
    }

    storage->current_size = 0;
}

