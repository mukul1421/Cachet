#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "../include/remote.h"

int connect_remote_server(const char *host) {

    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, "80", &hints, &result);

    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    int remote_fd = socket(
        result->ai_family,
        result->ai_socktype,
        result->ai_protocol
    );

    if (remote_fd == -1) {
        perror("socket");
        freeaddrinfo(result);
        return -1;
    }

    if (connect(
        remote_fd,
        result->ai_addr,
        result->ai_addrlen
    ) == -1) {

        perror("connect");
        close(remote_fd);
        freeaddrinfo(result);
        return -1;
    }

    freeaddrinfo(result);

    return remote_fd;
}


int send_request_to_remote(int remote_fd, const char *request) {

    int bytes_sent = send(
        remote_fd,
        request,
        strlen(request),
        0
    );

    if (bytes_sent == -1) {
        perror("send");
        return -1;
    }

    return bytes_sent;
}


int receive_remote_response(int remote_fd, char *response, int response_size) {

    int total_received = 0;

    while (total_received < response_size - 1) {

        int bytes_received = recv(
            remote_fd,
            response + total_received,
            response_size - 1 - total_received,
            0
        );

        if (bytes_received == -1) {
            perror("recv");
            return -1;
        }

        if (bytes_received == 0) {
            break;
        }

        total_received += bytes_received;
    }

    response[total_received] = '\0';

    return total_received;
}