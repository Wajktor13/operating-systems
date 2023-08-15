#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_PREFIX_LENGTH 255


bool validate_input(int argc, char *argv[]){
    if (argc != 3){
        fprintf(stderr, "wrong number of arguments. 2 expected, got %d\n", argc - 1);
        return false;
    } else if (strlen(argv[2]) > MAX_PREFIX_LENGTH){
        fprintf(stderr, "input string has exceeded 255 characters length limit\n");
        return false;
    }
    return true;
}

bool file_starts_with_prefix(char *path_to_file, char *prefix){
    FILE *file = fopen(path_to_file, "r");
    if (file == NULL){
        fprintf(stderr, "cannot open file\n");
        return false;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if ((read = getline(&line, &len, file)) != -1) {
        if (strncmp(line, prefix, strlen(prefix)) == 0){
            return true;
        }
    }

    fclose(file);

    return false;
}

void handle_file(char *path_to_file, char *prefix){
    if (file_starts_with_prefix(path_to_file, prefix)){
        printf("%s %d\n", path_to_file, getpid());
    }
}

void handle_dir(char *path_to_dir, char *prefix, char *argv0){
    DIR *dir = opendir(path_to_dir);
    struct dirent *entry;
    char *new_path = malloc(PATH_MAX);

    if (dir == NULL){
        fprintf(stderr, "cannot open dir\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }

        strcpy(new_path, path_to_dir);
        strcat(new_path, "/");
        strcat(new_path, entry->d_name);
        
        if (fork() == 0){
            execl(argv0, argv0, new_path, prefix, NULL);
        }
    }

    closedir(dir);
    free(new_path);
}


int main(int argc, char *argv[]) {
    char *input_dir = argv[1];
    char *prefix = argv[2];
    char path[PATH_MAX];
    struct stat path_stats;

    if (!validate_input(argc, argv)){
        return 1;
    }

    if (realpath(input_dir, path) == NULL){
        fprintf(stderr, "invalid path\n");
        return 1;
    }

    if (stat(path, &path_stats) == -1){
        fprintf(stderr, "cannot read stats\n");
        return 1;
    }

    setbuf(stdout, NULL);

    if(S_ISDIR(path_stats.st_mode)){
        handle_dir(path, prefix, argv[0]);
    } else{
        handle_file(path, argv[2]);
    }

    while (wait(NULL) > 0);

    return 0;
}
