#include "shared.h"


char* nickname;
int sock;

int connect_web_socket(char* ipv4, int port) {
    struct sockaddr_in addr;

    if (memset(&addr, 0, sizeof addr) == NULL) {
        printf("[client %s] memset error: %s\n", nickname, strerror(errno));
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ipv4, &addr.sin_addr) <= 0) {
        printf("[client %s] ivalid ipv4 address: %s\n", nickname, ipv4);
        exit(1);
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        printf("[client %s] socket error: %s\n", nickname, strerror(errno));
        exit(1);
    }

    if (connect(sock, (struct sockaddr*) &addr, sizeof addr) == -1) {
        printf("[client %s] connect error: %s\n", nickname, strerror(errno));
        exit(1);
    }
    
    return sock;
}

int connect_unix_socket(char* user, char* path) {
    struct sockaddr_un addr, bind_addr;

    if(memset(&addr, 0, sizeof addr) == NULL){
        printf("[client %s] memset error: %s\n", nickname, strerror(errno));
        exit(1);
    }
    
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof addr.sun_path -1);

    bind_addr.sun_family = AF_UNIX;
    snprintf(bind_addr.sun_path, sizeof bind_addr.sun_path, "/tmp/%s%ld", user, time(NULL));

    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(sock == -1){
        printf("[client %s] socket error: %s\n", nickname, strerror(errno));
        exit(1);
    }

    if (bind(sock, (struct sockaddr*) &bind_addr, sizeof bind_addr) == -1) { 
        printf("[client %s] bind error: %s\n", nickname, strerror(errno));
        exit(1);
    }

    if (connect(sock, (struct sockaddr*) &addr, sizeof addr) == -1) {
        printf("[client %s] connect error: %s\n", nickname, strerror(errno));
        exit(1);
    }

    return sock;
}

void handle_SIGINT() {
    Message msg = { 
        .type = DISCONNECT_CLIENT
    };

    if (sendto(sock, &msg, sizeof msg, 0, NULL, 0) == -1) { 
        printf("[client %s] sendto error: %s\n", nickname, strerror(errno));
    }

    exit(0);
}

void handle_NICKNAME_UNAVAILABLE(){
    printf("[client %s] nickname unavailable\n", nickname);
    if(close(sock) == -1){
        printf("[client %s] close error: %s\n", nickname, strerror(errno));
        exit(1);
    }
    exit(0);
}

void handle_SERVER_FULL(){
    printf("[client %s] server full\n", nickname);
    if(close(sock) == -1){
        printf("[client %s] close error: %s\n", nickname, strerror(errno));
        exit(1);
    }
    exit(0);
}

void handle_GET(Message msg){
    printf("[client %s] received:\n%s\n", nickname, msg.text);
}

void handle_PING(Message msg){
    if (write(sock, &msg, sizeof msg) == -1) {
        printf("[client %s] write error: %s\n", nickname, strerror(errno));
    }
}

void handle_STOP(){
    printf("[client %s] disconnected\n", nickname);
    if(close(sock) == -1){
        printf("[client %s] close error: %s\n", nickname, strerror(errno));
        exit(1);
    }
    exit(0);
}

void handle_SERVER_DISCONNECTED(){
    printf("[client %s] server disconnected\n", nickname);
    exit(0);
}

int main(int argc, char** argv) {
    setbuf(stdout, NULL);
    signal(SIGINT, handle_SIGINT);

    if (argc > 2){
            nickname = argv[1];
    } else {
        puts("[client] invalid arguments. Try: ./client 'nick 'web|unix' 'ip port|path'\n");
        exit(1);
    }

    if (strcmp(argv[2], "web") == 0 && argc == 5) {
        sock = connect_web_socket(argv[3], atoi(argv[4]));

    } else if (strcmp(argv[2], "unix") == 0 && argc == 4) {
        sock = connect_unix_socket(nickname, argv[3]);

    } else {
        puts("[client %s] invalid arguments. Try: ./client 'nick 'web|unix' 'ip port|path'\n");
        exit(1);
    }

    if (sock == -1) {
        printf("[client %s] socket error: %s\n", nickname, strerror(errno));
        return 1;
    }

    Message msg = { 
        .type = CONNECT_CLIENT
    };
    strncpy(msg.nickname, nickname, MAX_MSG_TEXT_SIZE); 
    if (send(sock, &msg, sizeof msg, 0) == -1) {
        printf("[client %s] send error: %s\n", nickname, strerror(errno));
        return 1;
    }
    
    int epfd = epoll_create1(0);

    if (epfd == -1) {
        printf("[client %s] epoll_create1 error: %s\n", nickname, strerror(errno));
        return 1;
    }
    
    struct epoll_event stdin_event = { 
        .events = EPOLLIN | EPOLLPRI,
        .data = { 
            .fd = STDIN_FILENO 
        }
    };
    
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event) == -1) {
        printf("[client %s] epoll_ctl error: %s\n", nickname, strerror(errno));
        return 1;
    }

    struct epoll_event SOCKET_EVENT = { 
        .events = EPOLLIN | EPOLLPRI | EPOLLHUP,
        .data = { 
            .fd = sock 
        }
    };

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &SOCKET_EVENT) == -1) {
        printf("[client %s] epoll_ctl error: %s\n", nickname, strerror(errno));
        return 1;
    }

    struct epoll_event events[2];

    printf("[client %s] connected, waiting for commands\n", nickname);
    
    while (true){
        int n = epoll_wait(epfd, events, 2, 1);

        if (n == -1) {
            printf("[client %s] epoll_wait error: %s\n", nickname, strerror(errno));
            return 1;
        }
        
        for(int i = 0; i < n; i++) {
            if (events[i].data.fd == STDIN_FILENO) {
                char buffer[MAX_MSG_TEXT_SIZE] = {};

                int j = read(STDIN_FILENO, &buffer, MAX_MSG_TEXT_SIZE);

                if (j == -1) {
                    printf("[client %s] read error: %s\n", nickname, strerror(errno));
                    return 1;
                }

                buffer[j] = 0;

                char *replace = " \t\n";
                char *token;
                int ind = 0;
                token = strtok(buffer, replace);

                MessageType type = -1;
                char nickname2[MAX_MSG_TEXT_SIZE] = {};
                char text[MAX_MSG_TEXT_SIZE] = {};

                bool end = false;

                if (token == NULL)
                continue;

                while( token != NULL ) {                
                    if (ind == 0){
                        if (strcmp(token, "LIST") == 0) {
                            type = LIST;
                        }
                        else if (strcmp(token, "STOP") == 0) {
                            type = STOP;
                        }
                        else if (strcmp(token, "2ONE") == 0) {
                            type = TO_ONE;
                        }
                        else if (strcmp(token, "2ALL") == 0) {
                            type = TO_ALL;
                        }

                        else {
                            end = true;
                            printf("[client %s] unknown command: %s\n", nickname, token);
                            type = -1;
                        }
                    }

                    if (ind == 1){
                        if(memcpy(text, token, strlen(token)*sizeof(char)) == NULL){
                            printf("[client %s] memcpy error: %s\n", nickname, strerror(errno));
                        }
                        text[strlen(token)] = 0;
                    }

                    if (ind == 2){
                        if (memcpy(nickname2, token, strlen(token)*sizeof(char) ) == NULL){
                            printf("[client %s] memcpy error: %s\n", nickname, strerror(errno));
                        }
                        nickname2[strlen(token)] = 0;
                    }

                    if (ind == 3){
                        end = true;
                    }

                    if (end){
                        break;
                    }

                    ind++;
                    token = strtok( NULL, replace );
                }

                if (end){
                    continue;
                }

                Message msg;
                msg.type = type;
                if (memcpy(&msg.nickname2, nickname2, strlen(nickname2) + 1) == NULL){
                    printf("[client %s] memcpy error: %s\n", nickname, strerror(errno));
                }

                if (memcpy(&msg.text, text, strlen(text) + 1) == NULL){
                    printf("[client %s] memcpy error: %s\n", nickname, strerror(errno));
                }

                if (sendto(sock, &msg, sizeof msg, 0, NULL, 0) == -1) {           
                    printf("[client %s] sendto error: %s\n", nickname, strerror(errno));
                }

            } else { 
                Message msg;
                if (recvfrom(sock, &msg, sizeof msg, 0, NULL, 0) == -1) { 
                    printf("[client %s] recvfrom error: %s\n", nickname, strerror(errno));
                }

                if (msg.type == NICKNAME_UNAVAILABLE) {
                    handle_NICKNAME_UNAVAILABLE();

                } else if (msg.type == SERVER_FULL) {
                    handle_SERVER_FULL();

                } else if (msg.type == GET) {
                    handle_GET(msg);

                } else if (msg.type == PING) {
                    handle_PING(msg);

                } else if (msg.type == STOP) {
                    handle_STOP();

                } else if (events[i].events & EPOLLHUP) {
                    handle_SERVER_DISCONNECTED();
                }
            }
        }
    }
}