#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>

typedef enum {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
} LogLevel;

void log_message(LogLevel level, const char *fmt, ...);
void generate_client_uuid(char *buf);
int is_ascii(const char *s);
int read_exact(int sock, void *buf, int len);
int write_exact(int sock, const void *buf, int len);
int decode_remaining_length(const unsigned char *buf, int *len);
int encode_remaining_length(unsigned char *buf, int len);

#endif
