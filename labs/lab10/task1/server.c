#include "shared.h"


#define MAX_CLIENTS 16
#define PING_DELAY 16
#define find_client(init, cond) ({ int index = -1;  for (init) if (cond) { index = i; break; } index; })

typedef enum { 
    CS_EMPTY = 0,
    CS_INIT,
    CS_READY 
} ClientState;

typedef enum { 
    SOKET_EVENT,
    CLIENT_EVENT
} EventType;

struct Client {
    int fd;
    ClientState state;
    char nickname[16];
    char symbol;
    struct GameState* GameState;
    bool is_responding;
} clients[MAX_CLIENTS];
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
    if (write(client->fd, &msg, sizeof(msg)) == -1){
        printf("[server] error while sending message to client %s: %s\n", client->nickname, strerror(errno));
    }

    printf("[server] sent message of type %d to client %s: %s\n", type, client->nickname, text);
}

Client* create_client(int cfd) {
    pthread_mutex_lock(&server_mutex);

    int i = find_client(int i = 0; i < MAX_CLIENTS; i++, clients[i].state == CS_EMPTY);

    if (i == -1) return NULL;
    
    Client* client = &clients[i];
    
    client->fd = cfd;
    client->state = CS_INIT;
    client->is_responding = true;

    pthread_mutex_unlock(&server_mutex);

    return client;
}

void remove_client(Client* client_to_remove) {
    client_to_remove->state = CS_EMPTY;
    client_to_remove->nickname[0] = 0;
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, client_to_remove->fd, NULL) == -1) {
        printf("[server] error while removing client %s: %s\n", client_to_remove->nickname, strerror(errno));
    }
    if (close(client_to_remove->fd) == -1) {
        printf("[server] error while closing client %s: %s\n", client_to_remove->nickname, strerror(errno));
    }
}

void* ping(void* _) {
    static Message msg = { 
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
                    if (write(clients[i].fd, &msg, sizeof msg) == -1) {
                        printf("[server] error while pinging client %s: %s\n", clients[i].nickname, strerror(errno));
                    }

                } else {
                    remove_client(&clients[i]);
                }
            }
        }

        pthread_mutex_unlock(&server_mutex);
    }
}

void init_socket(int socket, void* addr, int addr_size) {
    if (bind(socket, (struct sockaddr*) addr, addr_size) == -1) {
        printf("[server] error while binding socket: %s\n", strerror(errno));
        exit(1);
    }
    if (listen(socket, MAX_CLIENTS) == -1) {
        printf("[server] error while listening on socket: %s\n", strerror(errno));
        exit(1);
    }
    struct epoll_event event = { 
        .events = EPOLLIN | EPOLLPRI 
    };
    EventData* event_data = event.data.ptr = malloc(sizeof *event_data);
    event_data->type = SOKET_EVENT;
    event_data->payload.socket = socket;
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

void client_message_handler(Client* client) {
    if (client->state == CS_INIT) {
        handle_CS_INIT(client);
    }

    else {
        Message msg;
        if (read(client->fd, &msg, sizeof msg) == -1) {
            printf("[server] error while reading message from client %s: %s\n", client->nickname, strerror(errno));
            remove_client(client);
            return;
        }

        printf("[server] received message from %s of type: %d, text: %s\n", client->nickname, (int)msg.type, msg.text);

        if ( msg.type == STOP || msg.type == DISCONNECT_CLIENT) {
            handle_STOP_and_DISCONNECT_CLIENT(client, msg.type);

        } else if (msg.type == PING) {
            handle_PING(client);

        } else if (msg.type == TO_ALL) {
            handle_TO_ALL(client, msg);

        } else if (msg.type == TO_ONE) {
            handle_TO_ONE(client, msg);

        } else if (msg.type == LIST) {
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

    int local_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (local_socket == -1) {
        printf("[server] error while creating unix socket: %s\n", strerror(errno));
        exit(1);
    }
    init_socket(local_socket, &local_socket_addr, sizeof local_socket_addr);

    int web_socket = socket(AF_INET, SOCK_STREAM, 0);
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
            EventData* data = events[i].data.ptr;

            if (data->type == CLIENT_EVENT) {
                if (events[i].events & EPOLLHUP) {
                    pthread_mutex_lock(&server_mutex);
                    remove_client(data->payload.client);
                    pthread_mutex_unlock(&server_mutex);

                }else{
                    client_message_handler(data->payload.client);
                }

            } else if (data->type == SOKET_EVENT) {
                int cfd = accept(data->payload.socket, NULL, NULL);
                if (cfd == -1) {
                    printf("[server] error while accepting connection: %s\n", strerror(errno));
                    exit(1);
                }
                Client* client = create_client(cfd);

                if (client == NULL) {
                    printf("[server] cannot add new client, server is full\n");
                    Message msg = { 
                        .type = SERVER_FULL
                    };

                    if (write(cfd, &msg, sizeof msg) == -1) {
                        printf("[server] error while writing message to client: %s\n", strerror(errno));
                    }

                    if (close(cfd) == -1) {
                        printf("[server] error while closing socket: %s\n", strerror(errno));
                    }
                    continue;
                }

                EventData* event_data = calloc(1, sizeof(event_data));
                if (event_data == NULL) {
                    printf("[server] error while allocating memory for event data: %s\n", strerror(errno));
                    exit(1);
                }

                event_data->payload.client = client;
                event_data->type = CLIENT_EVENT;
                struct epoll_event event = { 
                    .events = EPOLLIN | EPOLLET | EPOLLHUP,
                    .data = { event_data } 
                };

                if(epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &event) == -1) {
                    printf("[server] error while adding client to epoll: %s\n", strerror(errno));
                    exit(1);
                }
            }
        }
    }
}
