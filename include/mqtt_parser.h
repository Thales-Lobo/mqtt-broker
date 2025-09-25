#ifndef MQTT_PARSER_H
#define MQTT_PARSER_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    MQTT_PKT_CONNECT   = 1,
    MQTT_PKT_CONNACK   = 2,
    MQTT_PKT_PUBLISH   = 3,
    MQTT_PKT_SUBSCRIBE = 8,
    MQTT_PKT_SUBACK    = 9,
    MQTT_PKT_PINGREQ   = 12,
    MQTT_PKT_PINGRESP  = 13,
    MQTT_PKT_DISCONNECT= 14
} MqttPacketType;

typedef struct {
    MqttPacketType type;
    char topic[128];
    char payload[1024];
    char client_id[64];
} MqttPacket;

int mqtt_parse_packet(const uint8_t *buf, size_t len, MqttPacket *pkt);
int mqtt_encode_connack(uint8_t *buf, size_t maxlen);
int mqtt_encode_suback(uint8_t *buf, size_t maxlen, uint16_t packet_id);
int mqtt_encode_publish(uint8_t *buf, size_t maxlen, const char *topic, const char *payload, size_t payload_len);
int mqtt_encode_pingresp(uint8_t *buf, size_t maxlen);

#endif
