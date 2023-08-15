#include "shared.h"

#define EMPTY_ID -1
#define SERVER_LOGS_PATH "server_logs.txt"
#define LOG_BUFFER_SIZE 256

int clients_queues[MAX_CLIENTS];
char log_buffer[LOG_BUFFER_SIZE];
int server_queue_id;
Message msg_out;


int get_first_free_id() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients_queues[i] == EMPTY_ID) {
            return i + 1;
        }
    }
    return EMPTY_ID;
}

void add_log(char* log){
    FILE* file = fopen(SERVER_LOGS_PATH, "a");
    struct tm current_time = *localtime(&(time_t){time(NULL)});

    if(file == NULL){
        fprintf(stderr, "[server] error while opening server logs file\n");
        exit(1);
    }

    fprintf(file, "[%d-%d-%d %d:%d:%d] ", current_time.tm_year + 1900, current_time.tm_mon + 1, current_time.tm_mday,
     current_time.tm_hour, current_time.tm_min, current_time.tm_sec);
    
    fprintf(file, "%s\n", log);
    fclose(file);
}

void handle_init(Message msg_in){
    int id = get_first_free_id(clients_queues);

    if(id != EMPTY_ID) {
        msg_out.type = id;
        msg_out.successfully_connected = true;
        clients_queues[id - 1] = msg_in.client_queue_id;
    } else {
        msg_out.type = 1;
        msg_out.successfully_connected = false;
    }

    if(msgsnd(msg_in.client_queue_id, &msg_out, MESSAGE_SIZE, 0) == -1){
        fprintf(stderr, "[server] error while sending message to client\n");
        exit(1);
    }

    printf("[server] successfully connected with Client %d\n", id);

    snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] successfully connected with Client %d\n", id);
    add_log(log_buffer);
}

void handle_list(){
    printf("[server] List of registered clients:\n");
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients_queues[i] != EMPTY_ID) {
            printf("[server] client %d: %d\n", i + 1, clients_queues[i]);
        }
    }
}

void handle_to_all(Message msg_in){
    time_t current_time = time(NULL);

    msg_out.type = TO_ALL;
    msg_out.client_queue_id = msg_in.client_queue_id;
    strcpy(msg_out.text, msg_in.text);
    msg_out.timestamp = *localtime(&current_time);

    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients_queues[i] == EMPTY_ID) {
            continue;
        }

        if(msgsnd(clients_queues[i], &msg_out, MESSAGE_SIZE, 0) == -1){
            fprintf(stderr, "[server] error while sending message to client %d\n", i);
            exit(1);

        } else {
            printf("[server] 2ALL message sent to client %d\n", i + 1);
            snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] 2ALL message sent to client %d\n", i + 1);
            add_log(log_buffer);
        }
    }
}

void handle_to_one(Message msg_in){
    time_t current_time = time(NULL);

    int receiver_id = msg_in.client_queue_id;
    msg_out.type = TO_ONE;
    msg_out.client_queue_id = receiver_id;
    strcpy(msg_out.text, msg_in.text);
    msg_out.timestamp = *localtime(&current_time);

    if(msgsnd(clients_queues[receiver_id - 1], &msg_out, MESSAGE_SIZE, 0) == -1){
        fprintf(stderr, "[server] error while sending message to client %d\n", receiver_id);
        exit(1);

    } else {
        printf("[server] 2ONE message sent to client %d\n", receiver_id);
        snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] 2ONE message sent to client %d\n", receiver_id);
        add_log(log_buffer);
    }
}

void shutdown_server(){
    msg_out.type = STOP;

    printf("\n");

    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients_queues[i] == EMPTY_ID) {
            continue;
        }

        if(msgsnd(clients_queues[i], &msg_out, MESSAGE_SIZE, 0) == -1){
            fprintf(stderr, "[server] error while sending message to client %d\n", i);
            exit(1);
            
        } else {
            printf("[server] STOP message sent to client %d\n", i + 1);
            snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] STOP message sent to client %d\n", i + 1);
            add_log(log_buffer);
        }
    }

    printf("[server] server is shutting down...\n");

    snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] server is shutting down...\n");
    add_log(log_buffer);

    msgctl(server_queue_id, IPC_RMID, NULL);

    exit(1);
}


int main() {
    signal(SIGINT, shutdown_server);

    for(int i = 0; i < MAX_CLIENTS; i++) { 
        clients_queues[i] = EMPTY_ID; 
        }

    key_t server_queue_key = ftok(getenv("HOME"), SERVER_ID);
    server_queue_id = msgget(server_queue_key, IPC_CREAT | 0666);

    Message msg_in;

    printf("[server] started\n");
    snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] started\n");

    while(true) {
        printf("[server] waiting for messages from clients...\n");
        if(msgrcv(server_queue_id, &msg_in, MESSAGE_SIZE, -6, 0) == -1) {

        }
    
        switch(msg_in.type) {
            case INIT: ;
                printf("[server] INIT received\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] INIT received\n");
                add_log(log_buffer);

                handle_init(msg_in);

                printf("[server] INIT handled\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] INIT handled\n");
                add_log(log_buffer);
                break;

            case LIST: 
                printf("[server] LIST received\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] LIST received\n");
                add_log(log_buffer);

                handle_list();

                printf("[server] LIST handled\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] LIST handled\n");
                add_log(log_buffer);
                break;

            case TO_ALL:
                printf("[server] TO_ALL received\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] TO_ALL received\n");
                add_log(log_buffer);

                handle_to_all(msg_in);

                printf("[server] TO_ALL handled\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] TO_ALL handled\n");
                add_log(log_buffer);
                break;

            case TO_ONE:
                printf("[server] TO_ONE received\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] TO_ONE received\n");
                add_log(log_buffer);

                handle_to_one(msg_in);

                printf("[server] TO_ONE handled\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] To_ONE handled\n");
                add_log(log_buffer);
                break;

            case STOP:
                printf("[server] STOP received\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] STOP received\n");
                add_log(log_buffer);

                clients_queues[msg_in.client_queue_id - 1] = EMPTY_ID;

                printf("[server] Client %d has disconnected\n", msg_in.client_queue_id);
                printf("[server] STOP handled\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] STOP handled\n");
                add_log(log_buffer);
                break;

            default:
                printf("Cannot recognize received message\n");
                break;
        }
    }

    return 0;
}