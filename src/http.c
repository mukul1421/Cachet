#include <stdio.h>
#include <string.h>
#include "../include/http.h"

void parse_request(const char *request, char *host, char *path){

    char method[16];
    
    sscanf(request, "%15s %255s", method, path);

    char *host_start = strstr(request, "Host:");

     if (host_start != NULL) {
        sscanf(host_start, "Host: %255s", host);
    } else {
        host[0] = '\0';
    }

    printf("[HTTP] %s %s\n", method, path);
    printf("[HTTP] Host: %s\n", host);
}