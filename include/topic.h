#ifndef TOPIC_H
#define TOPIC_H

#include "client.h"



typedef struct Subscriber {
    Client *client;
    struct Subscriber *next;
} Subscriber;

typedef struct Topic {
    char name[128];
    Subscriber *subscribers;
    struct Topic *next;
} Topic;

void storage_save_topics(const Topic *topics);
void topic_add_subscriber(const char *topic_name, Client *client);
void topic_remove_subscriber(const char *topic_name, Client *client);
void topic_publish(const char *topic_name, const char *payload, int payload_len);
void topic_cleanup(void);

#endif
