#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ftw.h>
#include <stdbool.h>

int total_files_size = 0;


int files_seeker(const char* filepath, const struct stat* file_stats, int file_type){
    if (file_type == FTW_F){
        printf("name:%s\tsize:%ld\n", filepath, file_stats->st_size);
        total_files_size += file_stats->st_size;
    }    

    return 0;
}


int main(int argc, char** argv){
     if (argc != 2){
        fprintf(stderr, "[files_seeker] error: invalid number of arguments. %d were given, 1 is required.\n", argc - 1);

    } else if (true){
        if (ftw(argv[1], files_seeker, 1) == -1){
            fprintf(stderr, "[files_seeker] error: ftw() failed.\n");
            return -1;
        }
        printf("\nTotal size: %d\n", total_files_size);

    } else{
        fprintf(stderr, "[files_seeker] error: invalid arguments. Try: *String*\n");
        return -1;
    }
    
    return 0;
}
