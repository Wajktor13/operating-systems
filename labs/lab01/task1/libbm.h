#ifndef __LIBBM_H__
#define __LIBBM_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


typedef struct
{
    char** blocks;
    bool* occupied;
    size_t max_size;
    size_t current_size;
}libbmStorage;


libbmStorage libbm_create_storage(size_t size);

int libbm_wc_write_block(libbmStorage* storage, char* file_name);

char* libbm_get_block(libbmStorage* storage, size_t ind);

bool libbm_remove_block(libbmStorage* storage, size_t ind);

void libbm_free_blocks(libbmStorage* storage);

#endif
