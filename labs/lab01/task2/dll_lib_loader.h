#ifndef __DLL_LIB_LOADER_H__
#define __DLL_LIB_LOADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>

#include "libbm.h"

void* handle = NULL;

#ifdef USE_DLL

void load_lib(const char* filepath) {
    handle = dlopen(filepath, RTLD_LAZY);

    if (handle == NULL) {
        fprintf(stderr, "[dll_lib_loader] error: failed to load library: %s\n", filepath);
        return;
    }

    *(void **) (&libbm_create_storage) = dlsym(handle,"libbm_create_storage");
    *(void **) (&libbm_wc_write_block) = dlsym(handle,"libbm_wc_write_block");
    *(void **) (&libbm_get_block) = dlsym(handle,"libbm_get_block");
    *(void **) (&libbm_remove_block) = dlsym(handle,"libbm_remove_block");
    *(void **) (&libbm_free_blocks) = dlsym(handle,"libbm_free_blocks");
}

#else

void load_lib(const char* filepath) {}

#endif

#endif
