#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {
    if (argc != 2){
        fprintf(stderr, "wrong number of arguments. 1 expected, got %d\n", argc - 1);
        return 1;
    }

    int no_child_processes = atoi(argv[1]);
    int child_pid;

    for (int i = 0; i < no_child_processes; i++){
        child_pid = fork();
        if (child_pid == 0){
            printf("%d %d\n", (int)getppid(), (int)getpid());
            return 0;
        }
    } 

    while (wait(NULL) > 0){
        continue;
    }
    
    printf("%d\n", no_child_processes);

    return 0;
}