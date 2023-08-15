#include "shared.h"

#define MAX_INPUT_SIZE 64

int client_queue_id;
int server_queue_id;
int client_id;
Message msg_out;


int init_connection() {
    msg_out.type = INIT;
    msg_out.client_queue_id = client_queue_id;

    if(msgsnd(server_queue_id, &msg_out, MESSAGE_SIZE, 0) == -1){
        fprintf(stderr, "[client] error while sending message to server\n");
        exit(1);
    }

    fprintf(stdout, "[client] waiting for registration confirmation... \n");

    if(msgrcv(client_queue_id, &msg_out, MESSAGE_SIZE, 0, 0) == -1){
        fprintf(stderr, "[client] error while receiving message from server\n");
        exit(1);
    }

    if (msg_out.successfully_connected) {
        printf("[client] registered successfully\n");

    } else {
        printf("[client] registration failed. Number of clients has exceeded the limit (%d)\n", MAX_CLIENTS);
        exit(1);
    }

    return msg_out.type;
}

void handle_list() {
    msg_out.type = LIST;
    msg_out.client_queue_id = client_queue_id; 

    if (msgsnd(server_queue_id, &msg_out, MESSAGE_SIZE, 0) == -1) {
        fprintf(stderr, "[client] error while sending message to server\n");
        exit(1);
    }
}

void handle_2all(char *data) {
    msg_out.type = TO_ALL;
    msg_out.client_queue_id = client_queue_id; 
    strcpy(msg_out.text, data);

    if (msgsnd(server_queue_id, &msg_out, MESSAGE_SIZE, 0) == -1) {
        fprintf(stderr, "[client] error while sending message to server\n");
        exit(1);
    }
}

void handle_2one(char *data, int receiver_id) {
    msg_out.type = TO_ONE;
    msg_out.client_queue_id = receiver_id;
    strcpy(msg_out.text, data);

    if (msgsnd(server_queue_id, &msg_out, MESSAGE_SIZE, 0) == -1) {
        fprintf(stderr, "[client] error while sending message to server\n");
        exit(1);
    }
}

void handle_stop() {
    msg_out.type = STOP;
    msg_out.client_queue_id = client_id; 

    msgsnd(server_queue_id, &msg_out, MESSAGE_SIZE, 0);

    fprintf(stdout, "\n[client] shutting down...\n");

    msgctl(client_queue_id, IPC_RMID, NULL);

    exit(0);
}

void check_inbox() {
    while (true){
        if (msgrcv(client_queue_id, &msg_out, MESSAGE_SIZE, 0, IPC_NOWAIT) == -1) {
            if (errno != ENOMSG) {
                fprintf(stderr, "[client] error while receiving message from server\n");
                exit(1);
            }
            
            break;

        } else {
            switch (msg_out.type)
            {
            case TO_ALL:
                printf("[client] received 2ALL message from server with text: %s\n", msg_out.text);
                break;

            case TO_ONE:
                printf("[client] received 2ONE message from server with text: %s\n", msg_out.text);
                break;

            case STOP:
                printf("[client] received STOP message from server\n");
                handle_stop();
                break;

            default:
                break;
            }
        }
    }
}

int main() {
    signal(SIGINT, handle_stop);

    key_t client_queue_key = ftok(KEY_PATH, getpid());
    client_queue_id = msgget(client_queue_key, IPC_CREAT | 0666);
    if(client_queue_id == -1){
        fprintf(stderr, "[client] error while creating queue\n");
        exit(1);
    }

    key_t server_queue_key = ftok(KEY_PATH, SERVER_ID);
    server_queue_id = msgget(server_queue_key, 0);
    if (server_queue_id == -1) {
        fprintf(stderr, "[client] error while getting server queue\n");
        exit(1);
    }

    client_id = init_connection(client_queue_id, server_queue_id);

    setbuf(stdout, NULL);

    char *user_input = NULL;
    size_t len = 0;
    ssize_t read;

    while(true){
        printf("[client] enter command: ");
        
        if ((read = getline(&user_input, &len, stdin)) == -1) {
            fprintf(stderr, "[client] error while reading user input\n");
            exit(1);
        }

        user_input[read - 1] = '\0';

        if (strcmp(user_input, "") == 0){
            printf("[client] unknown command\n");
            check_inbox(client_queue_id);
            continue;
        }

        char *command = strtok(user_input, " \n");

        if (strcmp(command, "LIST") == 0){
            handle_list();
            fprintf(stdout, "[client] LIST message has been sent to server\n");

        } else if (strcmp(command, "2ALL") == 0){
            char *data = strtok(NULL, " \n");
            if (data == NULL) {
                printf("[client] 2ALL takes one argument\n");
                continue;
            }
            handle_2all(data);
            fprintf(stdout, "[client] 2ALL message has been sent to server\n");

        } else if (strcmp(command, "2ONE") == 0){
            char *receiver_id_str = strtok(NULL, " \n");
            if (receiver_id_str == NULL) {
                printf("[client] 2ONE takes two arguments\n");
                continue;
            }
            int receiver_id = atoi(receiver_id_str);
            
            char *data = strtok(NULL, " \n");
            if (data == NULL) {
                printf("[client] 2ONE takes two arguments\n");
                continue;
            }

            handle_2one(data, receiver_id);
            fprintf(stdout, "[client] 2ONE message has been sent to client %d\n", receiver_id);

        } else if (strcmp(command, "STOP") == 0){
            handle_stop();
            fprintf(stdout, "[client] STOP message has been sent to server\n");
        } else {
            printf("[client] unknown command\n");
        }

        check_inbox(client_queue_id);
    }

    return 0;
}