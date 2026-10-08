#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/cache.h"

#define CACHE_SIZE 10

CacheEntry cache[CACHE_SIZE];

CacheEntry *find_in_cache(const char *key) {

    for (int i = 0; i < CACHE_SIZE; i++) {

        if (cache[i].key != NULL &&
            strcmp(cache[i].key, key) == 0) {

            return &cache[i];
        }
    }

    return NULL;
}


void add_to_cache(const char *key, const char *response, int response_size) {

    for (int i = 0; i < CACHE_SIZE; i++) {

        if (cache[i].key == NULL) {

            cache[i].key = malloc(strlen(key) + 1);
            cache[i].response = malloc(response_size + 1);

            if (cache[i].key == NULL || cache[i].response == NULL) {
                printf("Memory allocation failed.\n");
                return;
            }

            strcpy(cache[i].key, key);

            memcpy(
                cache[i].response,
                response,
                response_size
            );

            cache[i].response[response_size] = '\0';

            cache[i].response_size = response_size;

            return;
        }
    }

    printf("Cache is full.\n");
}