#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include <time.h>

#include "shared.h"
#define WAIT_FOR_MSG_TIME 0.3


char client_queue_name[CLIENT_NAME_LEN];
mqd_t client_queue_key;
mqd_t server_queue_key;
int client_id;
char msg[MAX_MSG_LEN];


char get_random_char() {
    return rand() % ('Z' - 'A' + 1) + 'A';
}

void generate_random_client_queue_name() {
    client_queue_name[0] = '/';
    client_queue_name[CLIENT_NAME_LEN - 1] = '\0';
    for(int i = 1; i < CLIENT_NAME_LEN - 1; i++) client_queue_name[i] = get_random_char();
}

int init_connection() {
    char msg_out[MAX_MSG_LEN];
    strcpy(msg_out, client_queue_name);

    if(mq_send(server_queue_key, msg_out, MAX_MSG_LEN, INIT) == -1){
        fprintf(stderr, "[client] requesting connection failed\n");
        exit(1);
    }

    printf("[client] requesting connection...\n");

    unsigned int client_id;
    if(mq_receive(client_queue_key, msg_out, MAX_MSG_LEN, &client_id) == -1){
        fprintf(stderr, "[client] error receiving registration confirmation\n");
        exit(1);
    }

    if(client_id == 0) {
        fprintf(stderr, "[client] server is full. Max number of clients is %d\n", MAX_CLIENTS);
        exit(1);
    }

    printf("[client] Succesfully connected. Assigned ID: %d\n", client_id);

    return client_id;
}

void handle_list() {
    if (mq_send(server_queue_key, msg, MAX_MSG_LEN, LIST) == -1) {
        fprintf(stderr, "[client] error while sending message to server\n");
        exit(1);
    }
}

void handle_2all(char *data) {
    strcpy(msg, data);

    if (mq_send(server_queue_key, msg, MAX_MSG_LEN, TO_ALL) == -1) {
        fprintf(stderr, "[client] error while sending message to server\n");
        exit(1);
    }
}

void handle_2one(char* data){
    if(mq_send(server_queue_key, data, MAX_MSG_LEN, TO_ONE) == -1){
        fprintf(stderr, "[server] error while sending message to client %d\n", client_id);
        exit(1);
    } 
}

void handle_stop() {
    snprintf(msg, MAX_MSG_LEN, "%d", client_id);
    
    mq_send(server_queue_key, msg, MAX_MSG_LEN, STOP);

    mq_close(client_queue_key);

    fprintf(stdout, "\n[client] shutting down...\n");

    exit(0);
}

void check_inbox() {
    unsigned int type;
    
    mq_close(client_queue_key);
    if((client_queue_key = mq_open(client_queue_name, O_RDWR | O_NONBLOCK)) == -1){
        fprintf(stderr, "[client] error reading client queue\n");
        exit(1);
    }

    while (true){

        if (mq_receive(client_queue_key, msg, MAX_MSG_LEN, &type) == -1) {
            if (errno != EAGAIN && errno != ETIMEDOUT) {
                fprintf(stderr, "[client] error while receiving message from server\n");
                exit(1);
            }
            
            break;

        } else {
            switch (type)
            {
            case TO_ALL:
                printf("[client] received 2ALL message from server with text: %s\n", msg);
                break;

            case TO_ONE:
                printf("[client] received 2ONE message from server with text: %s\n", msg);
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

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG_IN_QUEUE;
    attr.mq_msgsize = MAX_MSG_LEN;

    mq_close(client_queue_key);
    if((client_queue_key = mq_open(client_queue_name, O_RDWR | O_CREAT,
     S_IRWXU | S_IRWXG | S_IRWXO, &attr)) == -1){
        fprintf(stderr, "[client] error reading client queue\n");
        exit(1);
    }
}


int main() {
    signal(SIGINT, handle_stop);

    srand(time(NULL));

    generate_random_client_queue_name();

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG_IN_QUEUE;
    attr.mq_msgsize = MAX_MSG_LEN;

    client_queue_key = mq_open(client_queue_name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, &attr);
    if(client_queue_key == -1){
        fprintf(stderr, "[client] error opening client queue\n");
        exit(1);
    }

    printf("[client] generated queue name: %s\n", client_queue_name);

    server_queue_key = mq_open(SERVER_QUEUE_NAME, O_RDWR);
    if(server_queue_key == -1){
        fprintf(stderr, "[client] error opening server queue\n");
        exit(1);
    }

    client_id = init_connection();

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
            check_inbox();
            continue;
        }

        char *command = strtok(user_input, " \n");

        if (strcmp(command, "LIST") == 0){
            handle_list();
            fprintf(stdout, "[client] LIST message has been sent to server\n");

        } 
        else if (strcmp(command, "2ALL") == 0){
            char *data = strtok(NULL, " \n");
            if (data == NULL) {
                printf("[client] 2ALL takes one argument\n");
                continue;
            }
            handle_2all(data);
            fprintf(stdout, "[client] 2ALL message has been sent to server\n");

        } 
        else if (strcmp(command, "2ONE") == 0){
            char *receiver_id_str = strtok(NULL, " \n");
            if (receiver_id_str == NULL) {
                printf("[client] 2ONE takes two arguments\n");
                continue;
            }
            
            char *data = strtok(NULL, " \n");
            if (data == NULL) {
                printf("[client] 2ONE takes two arguments\n");
                continue;
            }

            strcat(receiver_id_str, data);

            handle_2one(receiver_id_str);
            fprintf(stdout, "[client] 2ONE message has been sent to client %s\n", receiver_id_str);

        } 

        else if (strcmp(command, "STOP") == 0){
            handle_stop();
            fprintf(stdout, "[client] STOP message has been sent to server\n");
        } else {
            printf("[client] unknown command\n");
        }

        check_inbox();
    }

    return 0;
}