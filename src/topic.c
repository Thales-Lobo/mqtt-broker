#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> 

#include "config.h"
#include "topic.h"
#include "mqtt_parser.h"
#include "utils.h"

void storage_save_topics(const Topic *topics) {
    FILE *f = fopen(TOPICS_FILE, "w");
    if (!f) {
        log_message(LOG_ERROR, "Error opening JSON status file (%s)", TOPICS_FILE);
        return;
    }

    fprintf(f, "{\n  \"topics\": [\n");

    const Topic *t = topics;
    int first_topic = 1;
    while (t) {
        if (!first_topic) fprintf(f, ",\n");
        first_topic = 0;

        fprintf(f, "    {\n      \"name\": \"%s\",\n      \"subscribers\": [", t->name);

        Subscriber *s = t->subscribers;
        int first_sub = 1;
        while (s) {
            if (!first_sub) fprintf(f, ", ");
            first_sub = 0;

            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &s->client->addr.sin_addr, ip_str, sizeof(ip_str));

            fprintf(f, "{ \"sock\": %d, \"client_id\": \"%s\", \"ip\": \"%s\", \"port\": %d }",
                    s->client->sock,
                    s->client->client_id,
                    ip_str,
                    ntohs(s->client->addr.sin_port));

            s = s->next;
        }

        fprintf(f, "]\n    }");
        t = t->next;
    }

    fprintf(f, "\n  ]\n}\n");

    fclose(f);
}

Topic *storage_load_topics(void) {
    FILE *f = fopen(TOPICS_FILE, "r");
    if (!f) {
        log_message(LOG_WARNING, "State file %s not found. No topics loaded.", TOPICS_FILE);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(f);
        log_message(LOG_ERROR, "Failed to allocate memory for reading the state");
        return NULL;
    }

    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    Topic *topics = NULL;
    char *pos = buf;

    while ((pos = strstr(pos, "\"name\":"))) {
        pos += 7;
        while (*pos == ' ' || *pos == '\"' || *pos == ':') pos++;
        char topic_name[128];
        sscanf(pos, "%127[^\"]", topic_name);

        Topic *t = malloc(sizeof(Topic));
        strncpy(t->name, topic_name, sizeof(t->name));
        t->subscribers = NULL;
        t->next = topics;
        topics = t;

        char *subs = strstr(pos, "\"subscribers\"");
        if (subs) {
            char *start = strchr(subs, '[');
            char *end = strchr(subs, ']');
            if (start && end && end > start) {
                char subs_buf[4096];
                strncpy(subs_buf, start + 1, end - start - 1);
                subs_buf[end - start - 1] = '\0';

                char *token = strtok(subs_buf, "{");
                while (token) {
                    if (strchr(token, '}')) {
                        int sock = 0, port = 0;
                        char client_id[64] = {0};
                        char ip[INET_ADDRSTRLEN] = {0};

                        sscanf(token, " \"sock\": %d , \"client_id\": \"%63[^\"]\" , \"ip\": \"%15[^\"]\" , \"port\": %d",
                               &sock, client_id, ip, &port);

                        Client *c = malloc(sizeof(Client));
                        if (c) {
                            c->sock = sock;
                            strncpy(c->client_id, client_id, sizeof(c->client_id));
                            c->addr.sin_family = AF_INET;
                            inet_pton(AF_INET, ip, &c->addr.sin_addr);
                            c->addr.sin_port = htons(port);
                            c->next = NULL;

                            Subscriber *s = malloc(sizeof(Subscriber));
                            if (s) {
                                s->client = c;
                                s->next = t->subscribers;
                                t->subscribers = s;
                            }
                        }
                    }
                    token = strtok(NULL, "{");
                }
            }
        }
    }

    free(buf);
    
    return topics;
}

static Topic *find_or_create_topic(const char *topic_name) {
    Topic *topics = storage_load_topics();
    Topic *t = topics;

    while (t) {
        if (strcmp(t->name, topic_name) == 0) {
            log_message(LOG_DEBUG, "Topic '%s' already exists", topic_name);
            return topics;
        }
        t = t->next;
    }

    Topic *new_topic = malloc(sizeof(Topic));
    if (!new_topic) {
        log_message(LOG_ERROR, "Failed to allocate memory for topic '%s'", topic_name);
        return topics;
    }
    strncpy(new_topic->name, topic_name, sizeof(new_topic->name) - 1);
    new_topic->name[sizeof(new_topic->name) - 1] = '\0';
    new_topic->subscribers = NULL;
    new_topic->next = topics;
    topics = new_topic;

    log_message(LOG_INFO, "New topic created: '%s'", topic_name);
    storage_save_topics(topics);

    return topics;
}

void topic_add_subscriber(const char *topic_name, Client *client) {
    Topic *topics = find_or_create_topic(topic_name);
    Topic *t = topics;

    while (t && strcmp(t->name, topic_name) != 0) {
        t = t->next;
    }
    if (!t) return;

    Subscriber *s = malloc(sizeof(Subscriber));
    if (!s) return;
    s->client = client;
    s->next = t->subscribers;
    t->subscribers = s;

    log_message(LOG_INFO, "Customer %s subscribed to the topic %s", client->client_id, topic_name);
    storage_save_topics(topics);
}

void topic_remove_subscriber(const char *topic_name, Client *client) {
    Topic *topics = storage_load_topics();
    Topic *t = topics;
    while (t) {
        if (strcmp(t->name, topic_name) == 0) {
            Subscriber **prev = &t->subscribers;
            Subscriber *s = t->subscribers;
            while (s) {
                if (s->client && strcmp(s->client->client_id, client->client_id) == 0) {
                    *prev = s->next;
                    free(s);
                    log_message(LOG_INFO, "Customer %s removed from topic %s", client->client_id, topic_name);
                    storage_save_topics(topics);
                    return;
                }
                prev = &s->next;
                s = s->next;
            }
        }
        t = t->next;
    }
}

void topic_publish(const char *topic_name, const char *payload, int payload_len) {
    Topic *t = storage_load_topics();

    while (t) {
        if (strcmp(t->name, topic_name) == 0) {
            Subscriber *s = t->subscribers;
            int count = 0;
            while (s) { count++; s = s->next; }

            log_message(LOG_INFO, "Posting to ‘%s’ for %d subscriber(s)", topic_name, count);

            s = t->subscribers;
            while (s) {
                unsigned char buf[2048];
                int len = mqtt_encode_publish(buf, sizeof(buf), topic_name, payload, payload_len);

                if (len > 0) {
                    log_message(LOG_DEBUG, "Sending PUBLISH for client %s (socket %d, %d bytes)",
                                s->client->client_id, s->client->sock, len);
                    write_exact(s->client->sock, buf, len);
                } else {
                    log_message(LOG_ERROR, "Failed to encode PUBLISH packet");
                }
                s = s->next;
            }
            return;
        }
        t = t->next;
    }

    log_message(LOG_WARNING, "No matching topics for '%s'", topic_name);
}

void topic_cleanup(void) {
    Topic *topics = storage_load_topics();
    while (topics) {
        Topic *next_t = topics->next;
        Subscriber *s = topics->subscribers;
        while (s) {
            Subscriber *next_s = s->next;
            if (s->client) free(s->client);
            free(s);
            s = next_s;
        }
        free(topics);
        topics = next_t;
    }

    FILE *f = fopen(TOPICS_FILE, "w");
    if (f) {
        fprintf(f, "{\n  \"topics\": []\n}\n");
        fclose(f);
        log_message(LOG_INFO, "File %s cleaned, no topics recorded.", TOPICS_FILE);
    } else {
        log_message(LOG_ERROR, "Could not clear file %s", TOPICS_FILE);
    }

    log_message(LOG_DEBUG, "Temporary memory state released and file cleaned up");
}