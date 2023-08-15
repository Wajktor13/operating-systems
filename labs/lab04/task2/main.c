#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

#define SIG SIGUSR1
#define MAX_CALLS_NODEFER 3

int calls_nodefer = 0;


void print_info(siginfo_t *info){
    printf("sender PID: %d\n", info->si_pid);
    printf("signo: %d\n", info->si_signo);
    printf("real UID: %d\n", info->si_uid);
    printf("errno: %d\n", info->si_errno);
    printf("err addr: %p\n", info->si_addr);
    printf("status: %d\n", info->si_status);
}

void handler_siginfo(int signo, siginfo_t *info, void *context){
    print_info(info);
}

void test_flag_siginfo(struct sigaction act){
    printf("\n*** Testing flag SA_SIGINFO ***\n\n");
    act.sa_sigaction = handler_siginfo;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIG, &act, NULL);

    raise(SIG);
}

void handler_nodefer(int signo, siginfo_t *info, void *context){
    int current_call = ++calls_nodefer;

    printf("\nCall no: %d\n", current_call);

    if (calls_nodefer < MAX_CALLS_NODEFER){
        raise(SIG);
    }

    printf("\nEnd of call no: %d\n", current_call);
}

void test_flag_nodefer(struct sigaction act){
    printf("\n\n*** Testing flag SA_NODEFER ***\n");
    printf("\nwithout SA_NODEFER:\n");

    act.sa_sigaction = handler_nodefer;
    sigemptyset(&act.sa_mask);
    sigaction(SIG, &act, NULL);

    raise(SIG);

    printf("\n\nwith SA_NODEFER:\n");
    calls_nodefer = 0;

    act.sa_sigaction = handler_nodefer;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER;
    sigaction(SIG, &act, NULL);

    raise(SIG);
}

void handler_nocldstop(int signo, siginfo_t *info, void *context){
    printf("recieved SIGCHLD (signo: %d)\n", signo);
}

void test_flag_nocldstop(struct sigaction act){
    int parent;

    printf("\n\n*** Testing flag SA_NOCLDSTOP ***\n");
    printf("\nwithout SA_NOCLDSTOP:\n");

    act.sa_sigaction = handler_nocldstop;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);

    parent = fork();
    if (parent == 0){
        raise(SIGSTOP);
        exit(0);
    } else {
        wait(NULL);

        printf("\nwith SA_NOCLDSTOP:\n");
        act.sa_sigaction = handler_nocldstop;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_NOCLDSTOP;
        sigaction(SIGCHLD, &act, NULL);

        parent = fork();
        if (parent == 0){
            raise(SIGSTOP);
            exit(0);
        }
    }
}

int main(){
    struct sigaction act;

    setbuf(stdout, NULL);
    
    test_flag_siginfo(act);
    test_flag_nodefer(act);
    test_flag_nocldstop(act);
}