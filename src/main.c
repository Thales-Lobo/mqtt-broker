/**
 * main.c
 * 
 * Entry point for the MQTT broker implementation (EP1).
 * 
 * Based on the echo server with pipe provided by Prof. Daniel Batista.
 * Adapted to initialize and run a broker MQTT 5.0.
 *
 * Responsibilities of this file:
 *  - Initialize the TCP server socket (bind, listen).
 *  - Accept multiple client connections.
 *  - Create a handler (child process/thread) for each client.
 *  - Delegate MQTT packet processing to broker and mqtt_parser.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include "broker.h"
#include "utils.h"
#include "config.h"

int listenfd;

void handle_sigint(int sig) {
    (void)sig;
    log_message(LOG_INFO, "Shutting down broker...");
    broker_cleanup();
    if (listenfd > 0) {
        close(listenfd);
    }
    exit(0);
}

int main(int argc, char **argv) {
    int port;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
        fprintf(stderr, "Example: %s 8000\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);

    broker_init();

    signal(SIGINT, handle_sigint);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        log_message(LOG_ERROR, "socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        log_message(LOG_ERROR, "bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, MAX_PENDING) == -1) {
        log_message(LOG_ERROR, "listen failed");
        exit(EXIT_FAILURE);
    }

    log_message(LOG_INFO, "Broker listening on port %d", port);

    for (;;) {
        int connfd;
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);

        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) == -1) {
            log_message(LOG_ERROR, "accept failed");
            continue;
        }

        log_message(LOG_INFO, "New connection from %s:%d: sock %d",
                inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), connfd);

        pid_t pid = fork();
        if (pid == 0) {
            close(listenfd);
            broker_handle_client(connfd);
            close(connfd);
            exit(0);
        } else if (pid < 0){
            log_message(LOG_ERROR, "fork failed");
            close(connfd);
        }
    }

    return 0;
}
