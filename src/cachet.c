#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "../include/http.h"
#include "../include/remote.h"
#include "../include/cache.h"


void *handle_client(void *arg) {

    int client_fd = *(int *)arg;
    free(arg);

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
            close(client_fd);
            return NULL;
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

    char host[256];
    char path[256];
    
    parse_request(buffer,host,path);

    char key[512];

    snprintf(key, sizeof(key), "%s%s", host, path);

    printf("Cache key: %s\n", key);

    char *cached_response = NULL;
    int cached_response_size = 0;
    
    int cache_hit = find_in_cache(
        key,
        &cached_response,
        &cached_response_size
    );
    
    if (cache_hit) {
    
        printf("Cache HIT.\n");
    
        int client_bytes_sent = send(
            client_fd,
            cached_response,
            cached_response_size,
            0
        );
    
        if (client_bytes_sent == -1) {
            perror("send");
            free(cached_response);
            close(client_fd);
            return NULL;
        }
    
        printf("Cached response sent to client.\n");
    
        free(cached_response);
        close(client_fd);
        return NULL;
    
    } else {
    
        printf("Cache MISS.\n");
    }

    int remote_fd = connect_remote_server(host);

    if (remote_fd != -1) {
    
        int bytes_sent = send_request_to_remote(remote_fd, buffer);
    
        if (bytes_sent != -1) {
            printf("Request forwarded to remote server.\n");
        }
    
        char response[8192];
    
        int response_size = receive_remote_response(
            remote_fd,
            response,
            sizeof(response)
        );
    
        if (response_size != -1) {
        
            printf("Response received from remote server.\n");
            printf("%s\n", response);

            add_to_cache(key, response, response_size);

            printf("Response added to cache.\n");
        
            int client_bytes_sent = send(
                client_fd,
                response,
                response_size,
                0
            );
        
            if (client_bytes_sent == -1) {
                perror("send");
                close(client_fd);
                return NULL;
            }
        
            printf("Response sent to client.\n");
        }
    
        close(remote_fd);
    }
    close(client_fd);
    return NULL;
}

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

    while (1) {

      
        int *client_socket = malloc(sizeof(int));

        if (client_socket == NULL) {
            perror("malloc");
            continue;
        }

        *client_socket = accept(server_fd, NULL, NULL);

        if (*client_socket == -1) {
            perror("accept");
            free(client_socket);
            continue;
        }

        printf("Client connected successfully.\n");

        pthread_t thread;

        int result = pthread_create(
            &thread,
            NULL,
            handle_client,
            client_socket
        );

        if (result != 0) {
            perror("pthread_create");
            close(*client_socket);
            free(client_socket);
            continue;
        }

        pthread_detach(thread);

    }

    
    return 0;
}