#ifndef CACHE_H
#define CACHE_H

typedef struct {
    char *key;
    char *response;
    int response_size;
} CacheEntry;

CacheEntry *find_in_cache(const char *key);

void add_to_cache(const char *key, const char *response, int response_size);

#endif