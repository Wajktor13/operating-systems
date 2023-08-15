#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_MSG_TEXT_SIZE 128


typedef enum {
    STOP,
    LIST,
    TO_ALL,
    TO_ONE,
    INIT,
    PING,
    NICKNAME_UNAVAILABLE,
    SERVER_FULL,
    DISCONNECT_CLIENT,
    GET,
    CONNECT_CLIENT,
} MessageType;

typedef struct {
    MessageType type;
    char text[MAX_MSG_TEXT_SIZE];
    char nickname[MAX_MSG_TEXT_SIZE];
    char nickname2[MAX_MSG_TEXT_SIZE];
} Message;
