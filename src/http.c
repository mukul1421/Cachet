#include <stdio.h>
#include <string.h>
#include "../include/http.h"

void parse_request(const char *request) {

    char method[16];
    char path[256];

    sscanf(request, "%15s %255s", method, path);

    printf("Method: %s\n", method);
    printf("Path: %s\n", path);
}