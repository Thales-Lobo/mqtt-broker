#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

typedef struct Client {
    int sock;
    char client_id[64];
    struct sockaddr_in addr;
    struct Client *next;
} Client;

Client *client_create(int sock, const char *id, struct sockaddr_in addr);

#endif
