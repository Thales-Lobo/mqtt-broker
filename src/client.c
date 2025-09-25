#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client.h"
#include "utils.h"

Client *client_create(int sock, const char *id, struct sockaddr_in addr) {
    Client *c = (Client *)malloc(sizeof(Client));
    if (!c) return NULL;

    c->sock = sock;
    strncpy(c->client_id, id ? id : "", sizeof(c->client_id) - 1);
    c->client_id[sizeof(c->client_id) - 1] = '\0';
    c->addr = addr;
    c->next = NULL;

    return c;
}

void client_destroy(Client *c) {
    if (!c) return;
    close(c->sock);
    free(c);
}
