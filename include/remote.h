#ifndef REMOTE_H
#define REMOTE_H

int connect_remote_server(const char *host);
int send_request_to_remote(int remote_fd, const char *request);
int receive_remote_response(int remote_fd, char *response, int response_size);

#endif