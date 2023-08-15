#include "shared.h"

#define EMPTY_ID -1
#define SERVER_LOGS_PATH "server_logs.txt"
#define LOG_BUFFER_SIZE 256


mqd_t clients_queue_keys[MAX_CLIENTS];
mqd_t server_queue_key;
char log_buffer[LOG_BUFFER_SIZE];
char msg[MAX_MSG_LEN];


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

int get_first_free_id() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients_queue_keys[i] == EMPTY_ID) {
            return i + 1;
        }
    }
    return -1;
}

void handle_init(char *msg_in){
    mqd_t client_queue_key;
    char msg_res[MAX_MSG_LEN];

    if((client_queue_key = mq_open(msg_in, O_RDWR)) == -1) {
        fprintf(stderr, "[server] error while opening client queue\n");
        exit(1);
    }

    int id = get_first_free_id();
    if(id > 0){
        clients_queue_keys[id - 1] = client_queue_key;        
    } else {
        id = 0;
    }

    if(mq_send(client_queue_key, msg_res, MAX_MSG_LEN, id) == -1) {
        fprintf(stderr, "[server] error while sending message to client\n");
        exit(1);
    }

    snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] successfully connected with Client %d\n", id);
    add_log(log_buffer);
}

void handle_list(){
    printf("[server] List of registered clients:\n");
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients_queue_keys[i] != EMPTY_ID) {
            printf("[server] client %d: %d\n", i + 1, clients_queue_keys[i]);
        }
    }
}

void handle_2all(char* msg_in){
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients_queue_keys[i] == EMPTY_ID) {
            continue;
        }

        if(mq_send(clients_queue_keys[i], msg_in, MAX_MSG_LEN, TO_ALL) == -1){
            fprintf(stderr, "[server] error while sending message to client %d\n", i + 1);
            exit(1);

        } else {
            printf("[server] 2ALL message sent to client %d\n", i + 1);
            snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] 2ALL message sent to client %d\n", i + 1);
            add_log(log_buffer);
        }
    }
}

void handle_2one(char* msg_in){
    int client_id = atoi(strndup(msg_in, 2));
    if(mq_send(clients_queue_keys[client_id - 1], msg_in, MAX_MSG_LEN, TO_ONE) == -1){
        fprintf(stderr, "[server] error while sending message to client %d\n", client_id);
        exit(1);

    } else {
        printf("[server] 2ONE message sent to client %d\n", client_id);
        snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] 2ONE message sent to client %d\n", client_id);
        add_log(log_buffer);
    }
}

void shutdown_server(){

    printf("\n");

    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients_queue_keys[i] == EMPTY_ID) {
            continue;
        }

        if(mq_send(clients_queue_keys[i], msg, MAX_MSG_LEN, STOP) == -1){
            fprintf(stderr, "[server] error while sending message to client %d\n", i);
            exit(1);
            
        } else {
            mq_close(clients_queue_keys[i]);
            printf("[server] STOP message sent to client %d\n", i + 1);
            snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] STOP message sent to client %d\n", i + 1);
            add_log(log_buffer);
        }
    }

    printf("[server] server is shutting down...\n");

    mq_close(server_queue_key);

    snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] server is shutting down...\n");
    add_log(log_buffer);

    exit(1);
}


int main() {
    signal(SIGINT, shutdown_server);

    setbuf(stdout, NULL);

    for(int i = 0; i < MAX_CLIENTS; i++) { 
        clients_queue_keys[i] = EMPTY_ID; 
        }

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG_IN_QUEUE;
    attr.mq_msgsize = MAX_MSG_LEN;

    server_queue_key = mq_open(SERVER_QUEUE_NAME, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, &attr);
    if(server_queue_key == -1){
        fprintf(stderr, "[server] error while creating server queue\n");
        exit(1);
    }

    char msg_in[MAX_MSG_LEN];
    unsigned int type;

    printf("[server] started\n");
    printf("[server] server queue name: %s\n", SERVER_QUEUE_NAME);

    while(true) {
        printf("[server] waiting for messages from clients...\n");

        if(mq_receive(server_queue_key, msg_in, MAX_MSG_LEN, &type) == -1){
            fprintf(stderr, "[server] error while receiving message\n");
            exit(1);
        };

        switch (type)
        {
            case INIT:
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

                handle_2all(msg_in);

                printf("[server] TO_ALL handled\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] TO_ALL handled\n");
                add_log(log_buffer);
                break;

            case TO_ONE:
                printf("[server] TO_ONE received\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] TO_ONE received\n");
                add_log(log_buffer);

                handle_2one(msg_in);

                printf("[server] TO_ONE handled\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] To_ONE handled\n");
                add_log(log_buffer);
                break;

            case STOP:
                printf("[server] STOP received\n");
                snprintf(log_buffer, LOG_BUFFER_SIZE, "[server] STOP received\n");
                add_log(log_buffer);

                int client_id = atoi(msg_in);
                mq_close(clients_queue_keys[client_id - 1]);
                clients_queue_keys[client_id - 1] = EMPTY_ID;

                printf("[server] Client %d has disconnected\n", client_id);
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