#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>


void print_mails(char *command){
        FILE *pipe_out = popen(command, "r");
        char *line = malloc(2048);

        if (pipe_out == NULL){
            fprintf(stderr, "Error while opening pipe\n");
        }
        
        while (fgets(line, 2048, pipe_out) != NULL){
            printf("%s", line);
        }

        if (pclose(pipe_out) == -1){
            fprintf(stderr, "Error while closing pipe\n");
        }
        
        free(line);
}

void send_mail(char *to, char *subject, char *message){
    char *command = malloc(4 * sizeof(char) + strlen(to) + 1);

    strcpy(command, "mail ");
    strcat(command, to);

    FILE *pipe_in = popen(command, "w");

    if (pipe_in == NULL){
        free(command);
        fprintf(stderr, "Error while opening pipe\n");
    }

    fputs(subject, pipe_in);
    fputs(message, pipe_in);

    if (pclose(pipe_in) == -1){
        free(command);
        fprintf(stderr, "Error while closing pipe\n");
    }

    free(command);
}
    

int main(int argc, char *argv[]){
    if (argc == 2){
        if(strcmp(argv[1], "nadawca") == 0){
            print_mails("mail -H | sort -n -k 3");

        } else if (strcmp(argv[1], "data") == 0){
            print_mails("mail -H");

        } else{
            fprintf(stderr, "invalid argument: %s\n", argv[1]);
            return 1;
        }
        
    } else if (argc == 4){
        send_mail(argv[1], argv[2], argv[3]);

    } else{
        fprintf(stderr, "Wrong number of arguments. One or three expected, got %d\n", argc - 1);
        return 1;
    }

    return 0;
}