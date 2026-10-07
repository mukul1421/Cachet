#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int main() {

    int server_fd;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    printf("TCP socket created successfully.\n");

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        return 1;
    }

    printf("Socket bound successfully to port 8080.\n");

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        return 1;
    }

    printf("Server is listening on port 8080.\n");

    int client_fd;

    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        return 1;
    }

    printf("Client connected successfully.\n");


    char buffer[4096];
    int total_received = 0;

    while (total_received < sizeof(buffer) - 1) {

        int bytes_received = recv(
            client_fd,
            buffer + total_received,
            sizeof(buffer) - 1 - total_received,
            0
        );

        if (bytes_received == -1) {
            perror("recv");
            return 1;
        }

        if (bytes_received == 0) {
            break;
        }

        total_received += bytes_received;

        buffer[total_received] = '\0';

        if (strstr(buffer, "\r\n\r\n") != NULL) {
            break;
        }
    }

    printf("Received HTTP request:\n%s\n", buffer);


    char response[] = "Hello from Cachet!\n";

    int bytes_sent = send(client_fd, response, sizeof(response) - 1, 0);

    if (bytes_sent == -1) {
        perror("send");
        return 1;
    }

    printf("Response sent to client.\n");

    close(client_fd);
    close(server_fd);

    return 0;
}