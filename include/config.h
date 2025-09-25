#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define DEFAULT_PORT 8000
#define MAX_PENDING 10
#define MAX_TOPICS 256
#define MAX_CLIENTS 1024
#define MAX_TOPIC_NAME 128
#define MAX_PAYLOAD_SIZE 1024
#define DEBUG_ENABLED false
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_RESET   "\033[0m"

#define LOG_FILE "logs/broker.log"
#define TOPICS_FILE "state/topics_state.json"

#endif
