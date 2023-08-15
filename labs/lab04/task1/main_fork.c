#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

#define SIG SIGUSR1


bool parent = true;


void raise_signal(int signum){
    if (parent){
        printf("sending signal %d to parent...\n", signum);
    }else{
        printf("sending signal %d to child...\n", signum);
    }

    raise(signum);
}

void handler(int signum){
    if (parent){
        printf("parent received signal: %d\n", signum);

    }else{
        printf("child received signal: %d\n", signum);
    }
}

void set_parent(char *command){
    struct sigaction act;

    if(strcmp(command, "ignore") == 0){
        signal(SIG, SIG_IGN);

    }else if(strcmp(command, "handler") == 0){
        signal(SIG, handler);

    }else if(strcmp(command, "mask") == 0 || strcmp(command, "pending") == 0){
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, SIG);
        sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

    } else{
        fprintf(stderr, "unknown command\n");
    }
}

int main(int argc, char *argv[]) {
    sigset_t sigset;

    setbuf(stdout, NULL);

    if (argc != 2){
        fprintf(stderr, "wrong number of arguments\n");
    }

    set_parent(argv[1]);

    if (strcmp(argv[1], "pending") != 0){
        parent = fork();
        raise_signal(SIG);  
    }

    else{
        raise_signal(SIG);
        parent = fork();

        sigpending(&sigset);
        
        if (!parent){
            if (sigismember(&sigset, SIG)){
                printf("signal pending in child\n");

            }else{
                printf("signal not pending in child\n");
            }
        }
    }

    return 0;
}