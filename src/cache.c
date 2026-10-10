#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "../include/cache.h"

#define CACHE_SIZE 10

CacheEntry cache[CACHE_SIZE];

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

int find_in_cache(const char *key, char **response, int *response_size) {

        printf("Trying to lock cache...\n");
        pthread_mutex_lock(&cache_mutex);
        printf("Cache lock acquired.\n");

    for (int i = 0; i < CACHE_SIZE; i++) {

        if (cache[i].key != NULL &&
            strcmp(cache[i].key, key) == 0) {

            *response = malloc(cache[i].response_size);

            if (*response == NULL) {
                pthread_mutex_unlock(&cache_mutex);
                return 0;
            }

            memcpy(
                *response,
                cache[i].response,
                cache[i].response_size
            );

            *response_size = cache[i].response_size;

            pthread_mutex_unlock(&cache_mutex);

            return 1;
        }
    }

    printf("Releasing cache lock.\n");
    pthread_mutex_unlock(&cache_mutex);

    return 0;
}


void add_to_cache(const char *key, const char *response, int response_size) {

    pthread_mutex_lock(&cache_mutex);

    for (int i = 0; i < CACHE_SIZE; i++) {

        if (cache[i].key == NULL) {

            cache[i].key = malloc(strlen(key) + 1);
            cache[i].response = malloc(response_size + 1);

            if (cache[i].key == NULL || cache[i].response == NULL) {
                printf("Memory allocation failed.\n");
                pthread_mutex_unlock(&cache_mutex);
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
            pthread_mutex_unlock(&cache_mutex);

            return;
        }

        
    }
    
    printf("Cache is full.\n");
    
    pthread_mutex_unlock(&cache_mutex);
}