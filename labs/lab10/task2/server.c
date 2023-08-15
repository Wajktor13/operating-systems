#include "shared.h"


#define MAX_CLIENTS 16
#define PING_DELAY 16
#define find_client(init, cond) ({ int index = -1;  for (init) if (cond) { index = i; break; } index; })

typedef enum { 
    CS_EMPTY = 0,
    CS_INIT,
    CS_READY,
    CS_WAITING,
} ClientState;

typedef enum { 
    SOCKET_EVENT,
    CLIENT_EVENT
} EventType;

union addr {
    struct sockaddr_un local_socket_addr;
    struct sockaddr_in web_socket_addr;
};
typedef struct sockaddr* saddr;

struct Client {
    union addr addr;
    int sock, addr_size;
    struct Client *peer;
    int fd;
    ClientState state;
    char nickname[16];
    char symbol;
    struct GameState* GameState;
    bool is_responding;
} clients[MAX_CLIENTS], *waiting_client = NULL;
typedef struct Client Client;

typedef union { 
    Client* client;
    int socket; 
} Payload;

typedef struct {
    EventType type;
    Payload payload;
} EventData;


pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;
int epfd;

void send_msg_to_client(Client* client, MessageType type, char *text) {
    Message msg;
    msg.type = type;
    memcpy(&msg.text, text, MAX_MSG_TEXT_SIZE*sizeof(char));
  
    if (sendto(client->sock, &msg, sizeof msg, 0, (saddr) &client->addr, client->addr_size) == -1) { 
        printf("[server] error while sending message to client %s: %s\n", client->nickname, strerror(errno));
    }

    printf("[server] sent message of type %d to client %s: %s\n", type, client->nickname, text);
}

void create_client(union addr* addr, socklen_t addr_size, int sock, char* nickname) {
    pthread_mutex_lock(&server_mutex);

    int ind = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].state == CS_EMPTY) {
            ind = i;
        } else if(strncmp(clients[i].nickname, nickname, sizeof clients->nickname) == 0) {
            pthread_mutex_unlock(&server_mutex);
            Message msg = { 
                .type = NICKNAME_UNAVAILABLE 
            };
            printf("[server] nickname is unavailable: %s\n", nickname);
            if (sendto(sock, &msg, sizeof msg, 0, (saddr) addr, addr_size) == -1) { 
                printf("[server] error while sending message to client %s: %s\n", nickname, strerror(errno));
            }

            return;
        }
    }

    if (ind == -1){
        pthread_mutex_unlock(&server_mutex);
        Message msg = { 
            .type = SERVER_FULL 
        };
        printf("[server] cannot add new client, server is full\n");
        if (sendto(sock, &msg, sizeof msg, 0, (saddr) addr, addr_size) == -1) { 
            printf("[server] error while sending message to client %s: %s\n", nickname, strerror(errno));
        }

        return;
    }
    
    Client* client = clients + ind;
    if (memcpy(&client->addr, addr, addr_size) == NULL) {
        printf("[server] error while copying client address: %s\n", strerror(errno));
        exit(1);
    }
    client->addr_size = addr_size;
    client->sock = sock;
    client->state = CS_WAITING;
    client->is_responding = true;

    if (memset(&client->nickname, 0, sizeof client->nickname) == NULL) {
        printf("[server] error while clearing client nickname: %s\n", strerror(errno));
        exit(1);
    }
    
    strncpy(client->nickname, nickname, sizeof client->nickname - 1);

    pthread_mutex_unlock(&server_mutex);
}

void remove_client(Client* client_to_remove) {
    Message msg = { 
        .type = DISCONNECT_CLIENT 
    };

    if (sendto(client_to_remove->sock, &msg, sizeof msg, 0, (saddr) &client_to_remove->addr, client_to_remove->addr_size) == -1) { 
        printf("[server] error while sending message to client %s: %s\n", client_to_remove->nickname, strerror(errno));
    }

    if (memset(&client_to_remove->addr, 0, sizeof client_to_remove->addr) == NULL) {
        printf("[server] error while clearing client: %s\n", strerror(errno));
    }

    client_to_remove->state = CS_EMPTY;
    client_to_remove->nickname[0] = 0;
    client_to_remove->sock = 0;
}

void* ping(void* _) {
    const static Message msg = { 
        .type = PING 
    };

    while (true) {
        sleep(PING_DELAY);
        pthread_mutex_lock(&server_mutex);

        printf("[server] pinging clients...\n");
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].state != CS_EMPTY) {
                if (clients[i].is_responding) {
                    clients[i].is_responding = false;
                    if (sendto(clients[i].sock, &msg, sizeof msg, 0, (saddr) &clients[i].addr, clients[i].addr_size) == -1) { 
                        printf("[server] error while sending message to client %s: %s\n", clients[i].nickname, strerror(errno));
                    }

                } else {
                    remove_client(&clients[i]);
                }
            }
        }

        pthread_mutex_unlock(&server_mutex);
    }

    return NULL;
}

void init_socket(int socket, void* addr, int addr_size) {
    if (bind(socket, (struct sockaddr*) addr, addr_size) == -1) {
        printf("[server] error while binding socket: %s\n", strerror(errno));
        exit(1);
    }

    struct epoll_event event = { 
        .events = EPOLLIN | EPOLLPRI,
        .data = { .fd = socket }
    };

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, socket, &event) == -1) {
        printf("[server] error while adding socket to epoll: %s\n", strerror(errno));
        exit(1);
    }
}

void create_message_text(char* out_text, char* nickname, char* text) {
    strcat(out_text, nickname);
    strcat(out_text, ": ");
    strcat(out_text, text);
}

void handle_CS_INIT(Client *client) {
    int nickname_size = read(client->fd, client->nickname, sizeof client->nickname - 1);
    if (nickname_size == -1) {
        printf("[server] error while reading nickname from client: %s\n", strerror(errno));
        remove_client(client);
        return;
    }
    
    int k = client - clients;

    pthread_mutex_lock(&server_mutex);

    if (find_client(int i = 0; i < MAX_CLIENTS; i++, i != k &&
        strncmp(client->nickname, clients[i].nickname, sizeof clients->nickname) == 0) == -1) {
        client->nickname[nickname_size] = '\0';
        client->state = CS_READY;
        printf("[server] new client connected: %s\n", client->nickname);

    } else {
        Message msg = { 
            .type = NICKNAME_UNAVAILABLE 
        };
        printf("[server] nickname is unavailable: %s\n", client->nickname);
        if (write(client->fd, &msg, sizeof msg) == -1) {
            printf("[server] error while sending message to client %s: %s\n", client->nickname, strerror(errno));
        }
        strcpy(client->nickname, "x");
        remove_client(client);
    }

    pthread_mutex_unlock(&server_mutex);
}

void handle_STOP_and_DISCONNECT_CLIENT(Client* client, MessageType type) {
    pthread_mutex_lock(&server_mutex);
    printf("[server] removed client: %s\n", client->nickname);
    remove_client(client);
    pthread_mutex_unlock(&server_mutex);
}

void handle_PING(Client* client) {
    pthread_mutex_lock(&server_mutex);
    printf("[server] received pong from: %s\n", client->nickname);
    client->is_responding = true;
    pthread_mutex_unlock(&server_mutex);
}

void handle_TO_ALL(Client* client, Message msg) {
    char out_text[MAX_MSG_TEXT_SIZE] = "";
    create_message_text(out_text, client->nickname, msg.text);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].state != CS_EMPTY) {
            send_msg_to_client(clients+i, GET, out_text);
        }
    }
}

void handle_TO_ONE(Client* client, Message msg) {
    char out_text[MAX_MSG_TEXT_SIZE] = "";
    create_message_text(out_text, client->nickname, msg.text);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].state != CS_EMPTY) {
            if (strcmp(clients[i].nickname, msg.nickname2) == 0) {
                send_msg_to_client(clients+i, GET, out_text);
            }
        }
    }
}

void handle_LIST(Client* client) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].state != CS_EMPTY)
            send_msg_to_client(client, GET, clients[i].nickname);
    }
}

void client_message_handler(Client* client, Message *msg) {
    if (client->state == CS_INIT) {
        handle_CS_INIT(client);
    }

    else {
        printf("[server] received message from %s of type: %d, text: %s\n", client->nickname, (int)msg->type, msg->text);

        if ( msg->type == STOP || msg->type == DISCONNECT_CLIENT) {
            handle_STOP_and_DISCONNECT_CLIENT(client, msg->type);

        } else if (msg->type == PING) {
            handle_PING(client);

        } else if (msg->type == TO_ALL) {
            handle_TO_ALL(client, *msg);

        } else if (msg->type == TO_ONE) {
            handle_TO_ONE(client, *msg);

        } else if (msg->type == LIST) {
            handle_LIST(client);
        } 
    }
}

int main(int argc, char** argv) {
    setbuf(stdout, NULL);

    if (argc != 3) {
        printf("[server] invalid arguments. Try: ./server 'port' 'path'\n");
        return 0;
    }

    int port = atoi(argv[1]);
    char* unix_socket_path = argv[2];

    printf("[server] listening on port %d and unix socket '%s'\n", port, unix_socket_path);

    epfd = epoll_create1(0);

    if (epfd == -1) {
        printf("[server] error while creating epoll: %s\n", strerror(errno));
        exit(1);
    }

    struct sockaddr_in web_socket_addr = {
        .sin_family = AF_INET, .sin_port = htons(port),
        .sin_addr = { 
            .s_addr = htonl(INADDR_ANY) 
        },
    };

    struct sockaddr_un local_socket_addr = { 
        .sun_family = AF_UNIX 
    };
    strncpy(local_socket_addr.sun_path, unix_socket_path, sizeof local_socket_addr.sun_path);

    unlink(unix_socket_path);

    int local_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (local_socket == -1) {
        printf("[server] error while creating unix socket: %s\n", strerror(errno));
        exit(1);
    }
    init_socket(local_socket, &local_socket_addr, sizeof local_socket_addr);

    int web_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (web_socket == -1) {
        printf("[server] error while creating web socket: %s\n", strerror(errno));
        exit(1);
    }
    init_socket(web_socket, &web_socket_addr, sizeof web_socket_addr);

    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping, NULL);

    struct epoll_event events[10];

    while (true) {
        int n = epoll_wait(epfd, events, 10, -1);
        if (n == -1) {
            printf("[server] error while waiting for events: %s\n", strerror(errno));
            exit(1);
        }

        for(int i = 0; i < n; i++) {
            int sock = events[i].data.fd;
            Message msg;
            union addr addr;
            socklen_t addr_size = sizeof addr;
            recvfrom(sock, &msg, sizeof msg, 0, (saddr) &addr, &addr_size);

            if (msg.type == CONNECT_CLIENT){
                create_client(&addr, addr_size, sock, msg.nickname);
            } else {
                int client_index = find_client(int i = 0; i < MAX_CLIENTS; i++, memcmp(&clients[i].addr, &addr, addr_size) == 0);
                if (client_index != -1) {
                    client_message_handler(clients + client_index, &msg);
                }
            }
        }
    }
}
