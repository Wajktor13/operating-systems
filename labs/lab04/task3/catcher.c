#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define SIG SIGUSR1

typedef enum {
    PRINT_NUMBERS = 1,
    PRINT_TIME = 2,
    PRINT_REQUESTS_TIMER = 3,
    PRINT_TIME_LOOP = 4,
    EXIT_CATCHER = 5,
} work_mode;

work_mode current_work_mode = 1;
bool waiting_for_request = true;
int requests_counter = 0;


void handler(int signum, siginfo_t *signal_info, void *ucontext){
    int sender_pid = signal_info->si_pid;
    int request = signal_info->si_status;

    if (!waiting_for_request && current_work_mode != PRINT_TIME_LOOP){
        return;
    }

    if (request < 1 || request > 5){
        fprintf(stderr, "Signal: %d.Invalid request: %d. Valid request is an int between 1 and 5.\n", signum, request);
        exit(0);
    } else{
        printf("[catcher] received signal %d from %d with request %d\n", signum, sender_pid, request);
        current_work_mode = request;
        waiting_for_request = false;

        kill(sender_pid, SIG);
    }

    requests_counter++;
}

void set_sigaction(){
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIG, &act, NULL);
}

void print_numbers(){
    for (int i = 1; i <= 100; i++){
        printf("[catcher] %d\n", i);
    }
}

void print_time(){
    time_t current_time = time(NULL);
    printf("[catcher] %s", ctime(&current_time));
}

void print_time_loop(){
    while (current_work_mode == PRINT_TIME_LOOP){
        print_time();
        sleep(1);
    }
}

int main(){
    bool loop_flag = false;

    set_sigaction();

    printf("[catcher] waiting for sender request. PID: %d\n", getpid());

    while (true){
        if (waiting_for_request){
            continue;
        } else{
            switch (current_work_mode){
                case PRINT_NUMBERS:
                    print_numbers();
                    break;

                case PRINT_TIME:
                    print_time();
                    break;

                case PRINT_REQUESTS_TIMER:
                    printf("[catcher] requests counter: %d\n", requests_counter   );
                    break;

                case PRINT_TIME_LOOP:
                    print_time_loop();
                    loop_flag = true;
                    break;

                case EXIT_CATCHER:
                    printf("[catcher] exiting\n");
                    return 0;
            }

            if (loop_flag){
                loop_flag = false;
            } else{
                waiting_for_request = true;
            }
        }
    }

    return 0;
}
