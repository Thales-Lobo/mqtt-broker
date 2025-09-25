#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "broker.h"
#include "utils.h"
#include "config.h"

static bool broker_running = false;

void broker_init(void) {
    broker_running = true;
    log_message(LOG_INFO, "Broker initialized");
}

void broker_cleanup(void) {
    broker_running = false;
    topic_cleanup();
    log_message(LOG_INFO, "Broker cleaned up");
}

void broker_handle_client(int sock) {
    unsigned char buf[2048];
    ssize_t n;

    log_message(LOG_INFO, "Handling client on socket %d", sock);

    while (broker_running) {
        n = read(sock, buf, sizeof(buf));
        if (n <= 0) {
            log_message(LOG_INFO, "Client on socket %d disconnected", sock);
            close(sock);
            return;
        }

        MqttPacket pkt;
        if (mqtt_parse_packet(buf, n, &pkt) < 0) {
            log_message(LOG_ERROR, "Failed to parse MQTT packet");
            continue;
        }

        switch (pkt.type) {
            case MQTT_PKT_CONNECT: {
                log_message(LOG_INFO, "CONNECT received from client");
                unsigned char reply[16];
                int len = mqtt_encode_connack(reply, sizeof(reply));
                write_exact(sock, reply, len);
                break;
            }

            case MQTT_PKT_SUBSCRIBE: {
                log_message(LOG_INFO, "SUBSCRIBE to topic '%s'", pkt.topic);
                Client *c = client_create(sock, pkt.client_id, (struct sockaddr_in){0});
                topic_add_subscriber(pkt.topic, c);
                unsigned char reply[16];
                int len = mqtt_encode_suback(reply, sizeof(reply), 1);
                write_exact(sock, reply, len);
                break;
            }

            case MQTT_PKT_PUBLISH: {
                log_message(LOG_INFO, "PUBLISH to topic '%s' with payload '%s'", pkt.topic, pkt.payload);
                topic_publish(pkt.topic, pkt.payload, strlen(pkt.payload));
                break;
            }

            case MQTT_PKT_DISCONNECT: {
                log_message(LOG_INFO, "DISCONNECT from client on socket %d", sock);
                close(sock);
                return;
            }

            case MQTT_PKT_PINGREQ: {
                log_message(LOG_INFO, "PINGREQ received");
                unsigned char reply[8];
                int len = mqtt_encode_pingresp(reply, sizeof(reply));
                write_exact(sock, reply, len);
                break;
            }

            default:
                log_message(LOG_ERROR, "Unhandled MQTT packet type %d", pkt.type);
                break;
        }
    }

    close(sock);
}
