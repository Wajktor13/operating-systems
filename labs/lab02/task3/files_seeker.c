#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>


void files_seeker(){
    DIR *dir;
    struct dirent *ent;
    struct stat file_stats;
    long long total_files_size = 0;

    if ((dir = opendir (".")) == NULL) {
        fprintf(stderr, "[files_seeker] error: cannot open current directory\n");
    }
    
    while ((ent = readdir (dir)) != NULL) {
        stat(ent->d_name, &file_stats);

        if (!S_ISDIR(file_stats.st_mode)){
            printf ("name:%s\tsize:%ld\n", ent->d_name, file_stats.st_size);
            total_files_size += file_stats.st_size;
        }
    }

    printf("\nTotal files size: %lld\n", total_files_size);

    closedir(dir);
}


int main(){
    files_seeker();
    
    return 0;
}
