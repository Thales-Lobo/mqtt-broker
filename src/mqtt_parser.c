#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_parser.h"
#include "utils.h"

int mqtt_parse_packet(const uint8_t *buf, size_t len, MqttPacket *pkt) {
    if (len < 2) return -1;

    uint8_t packet_type = (buf[0] >> 4) & 0x0F;
    pkt->type = (MqttPacketType)packet_type;

    switch (pkt->type) {
        case MQTT_PKT_CONNECT: {
            log_message(LOG_DEBUG, "CONNECT packet received (len=%zu)\n", len);

            int pos = 2;

            int proto_len = (buf[pos] << 8) | buf[pos+1];
            pos += 2;
            log_message(LOG_DEBUG, "Protocol Name length=%d\n", proto_len);

            char proto_name[16] = {0};
            memcpy(proto_name, &buf[pos], proto_len);
            proto_name[proto_len] = '\0';
            pos += proto_len;
            log_message(LOG_DEBUG, "Protocol Name='%s'\n", proto_name);

            uint8_t proto_level = buf[pos++];
            log_message(LOG_DEBUG, "Protocol Level=%d\n", proto_level);

            uint8_t flags = buf[pos++];
            log_message(LOG_DEBUG, "Connect Flags=0x%02X\n", flags);

            uint16_t keepalive = (buf[pos] << 8) | buf[pos+1];
            pos += 2;
            log_message(LOG_DEBUG, "Keep Alive=%u\n", keepalive);

            int id_len = (buf[pos] << 8) | buf[pos+1];
            pos += 2;
            log_message(LOG_DEBUG, "Client ID length=%d\n", id_len);

            if (id_len == 0) {
                generate_client_uuid(pkt->client_id);
                log_message(LOG_DEBUG, "Empty Client ID, assigned='%s'\n", pkt->client_id);
            }

            log_message(LOG_DEBUG, "Parsed Client ID='%s'\n", pkt->client_id);

            break;
        }

        case MQTT_PKT_SUBSCRIBE: {
            if (len < 5) return -1;

            size_t pos = 2;

            if (pos + 2 > len) return -1;
            pos += 2;

            if (pos + 2 > len) return -1;
            size_t topic_len = (buf[pos] << 8) | buf[pos+1];
            pos += 2;

            if (topic_len >= sizeof(pkt->topic)) topic_len = sizeof(pkt->topic)-1;
            if (pos + topic_len > len) return -1;
            memcpy(pkt->topic, &buf[pos], topic_len);
            pkt->topic[topic_len] = '\0';
            pos += topic_len;

            if (pos >= len) return -1;

            break;
        }


        case MQTT_PKT_PUBLISH: {
            if (len < 4) return -1;
            size_t topic_len = (buf[2] << 8) | buf[3];
            if (topic_len >= sizeof(pkt->topic)) return -1;
            memcpy(pkt->topic, &buf[4], topic_len);
            pkt->topic[topic_len] = '\0';

            size_t payload_offset = 4 + topic_len;
            size_t payload_len = len - payload_offset;
            if (payload_len >= sizeof(pkt->payload)) return -1;
            memcpy(pkt->payload, &buf[payload_offset], payload_len);
            pkt->payload[payload_len] = '\0';
            break;
        }

        case MQTT_PKT_DISCONNECT:
        case MQTT_PKT_PINGREQ:
            break;

        default:
            return -1;
    }

    return 0;
}

int mqtt_encode_connack(uint8_t *buf, size_t maxlen) {
    if (maxlen < 4) return -1;
    buf[0] = 0x20;
    buf[1] = 0x02;
    buf[2] = 0x00;
    buf[3] = 0x00;
    return 4;
}

int mqtt_encode_suback(uint8_t *buf, size_t maxlen, uint16_t packet_id) {
    if (maxlen < 5) return -1;
    buf[0] = 0x90;
    buf[1] = 0x03;
    buf[2] = (packet_id >> 8) & 0xFF;
    buf[3] = packet_id & 0xFF;
    buf[4] = 0x00;
    return 5;
}

int mqtt_encode_publish(uint8_t *buf, size_t maxlen,
                        const char *topic, const char *payload,
                        size_t payload_len) {
    size_t topic_len = strlen(topic);
    size_t remaining_len = 2 + topic_len + payload_len;
    if (remaining_len + 2 > maxlen) return -1;

    buf[0] = 0x30;
    buf[1] = remaining_len;
    buf[2] = (topic_len >> 8) & 0xFF;
    buf[3] = topic_len & 0xFF;
    memcpy(&buf[4], topic, topic_len);
    memcpy(&buf[4 + topic_len], payload, payload_len);

    return 4 + topic_len + payload_len;
}

int mqtt_encode_pingresp(uint8_t *buf, size_t maxlen) {
    if (maxlen < 2) return -1;
    buf[0] = 0xD0;
    buf[1] = 0x00;
    return 2;
}
