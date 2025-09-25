#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <uuid/uuid.h>

#include "config.h"
#include "utils.h"

void generate_client_uuid(char *buf) {
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, buf);
}

void log_message(LogLevel level, const char *fmt, ...) {
    if (level == LOG_DEBUG && !DEBUG_ENABLED) {
        return;
    }

    FILE *f = fopen(LOG_FILE, "a+");
    if (!f) {
        perror("Error opening log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    const char *level_str;
    const char *color;
    switch (level) {
        case LOG_ERROR:
            level_str = "ERROR";
            color = COLOR_RED;
            break;
        case LOG_WARNING:
            level_str = "WARN";
            color = COLOR_YELLOW;
            break;
        case LOG_INFO:
            level_str = "INFO";
            color = COLOR_GREEN;
            break;
        case LOG_DEBUG:
        default:
            level_str = "DEBUG";
            color = COLOR_CYAN;
            break;
    }

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);

    fprintf(f, "[%s] [%s] ", timebuf, level_str);
    vfprintf(f, fmt, args);
    fprintf(f, "\n");

    printf("%s[%s] [%s] ", color, timebuf, level_str);
    vprintf(fmt, args_copy);
    printf("%s\n", COLOR_RESET);

    va_end(args_copy);
    va_end(args);

    fclose(f);
}

int is_ascii(const char *s) {
    while (*s) {
        if ((unsigned char)*s < 0x20 || (unsigned char)*s > 0x7E) {
            return 0;
        }
        s++;
    }
    return 1;
}

int read_exact(int sock, void *buf, int len) {
    int total = 0;
    while (total < len) {
        int n = read(sock, (char *)buf + total, len - total);
        if (n <= 0) return -1;
        total += n;
    }
    return total;
}

int write_exact(int sock, const void *buf, int len) {
    int total = 0;

    while (total < len) {
        int n = write(sock, (const char *)buf + total, len - total);
        if (n <= 0) return -1;
        total += n;
    }

    log_message(LOG_DEBUG, "Pacote enviado para socket %d", sock);

    return total;
}

int decode_remaining_length(const unsigned char *buf, int *len) {
    int multiplier = 1;
    int value = 0;
    int i = 0;
    unsigned char encoded;

    do {
        encoded = buf[i];
        value += (encoded & 127) * multiplier;
        multiplier *= 128;
        i++;
        if (i > 4) return -1;
    } while ((encoded & 128) != 0);

    *len = value;
    return i;
}

int encode_remaining_length(unsigned char *buf, int len) {
    int i = 0;
    do {
        unsigned char encoded = len % 128;
        len /= 128;
        if (len > 0) encoded |= 128;
        buf[i++] = encoded;
    } while (len > 0);
    return i;
}
