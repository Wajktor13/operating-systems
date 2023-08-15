#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <mqueue.h>
#include <time.h>

#define KEY_PATH getenv("HOME")
#define SERVER_ID 0
#define MAX_MESSAGE_TEXT_SIZE 256
#define MAX_CLIENTS 3


typedef enum MessageType {
    STOP= 1,
    LIST = 2,
    TO_ALL = 3,
    TO_ONE = 4,
    INIT = 5,
} MessageType;

typedef struct Message {
    long type;
    char text[MAX_MESSAGE_TEXT_SIZE];
    bool successfully_connected;
    int client_queue_id;
    key_t client_queue_key;
    struct tm timestamp;
} Message;

const int MESSAGE_SIZE = sizeof(Message) - sizeof(long);

#endif
