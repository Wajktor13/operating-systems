#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define SIG SIGUSR1

bool recieved_confirmation = false;


void handler(){
    recieved_confirmation = true;
}


int main(int argc, char *argv[]){
    if (argc < 3){
        fprintf(stderr, "[sender] too few arguments. At least 2 expected, got %d\n", argc - 1);
        return 1;
    }

    signal(SIG, handler);

    int catcher_pid = atoi(argv[1]);

    for (int i = 2; i < argc; i++){
        int request = atoi(argv[i]);
        sigval_t request_sigval = {request};
        sigqueue(catcher_pid, SIG, request_sigval);
        printf("[sender] sent request %d to catcher with PID %d\n", request, catcher_pid);
        
        time_t start = clock();
        while (clock() - start < CLOCKS_PER_SEC)
        {
            continue;
        }

        if (recieved_confirmation){
            printf("[sender] received confirmation from catcher\n");
            recieved_confirmation = false;
        } else{
            printf("[sender] no confirmation received from catcher\n");
        }
        
    }

    return 0;
}
