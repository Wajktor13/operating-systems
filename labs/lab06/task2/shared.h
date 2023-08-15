#ifndef SHARED_H
#define SHARED_H

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

#define MAX_CLIENTS 3
#define CLIENT_NAME_LEN 32
#define MAX_MSG_IN_QUEUE 10
#define MAX_MSG_LEN 256

const char* SERVER_QUEUE_NAME = "/SERVER";


typedef enum MessageType {
    STOP= 1,
    LIST = 2,
    TO_ALL = 3,
    TO_ONE = 4,
    INIT = 5,
} MessageType;

#endif
