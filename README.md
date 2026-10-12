# Cachet

Cachet is a lightweight multithreaded HTTP proxy server written in C.

It accepts client HTTP requests, checks an in-memory cache, forwards cache misses to a remote server, stores the response, and serves repeated requests directly from the cache.

The project focuses on TCP networking, HTTP parsing, multithreading, synchronization, dynamic memory management, and LRU caching.

## Architecture

```text
                         +------------------+
                         |      Client      |
                         +--------+---------+
                                  |
                                  v
                         +------------------+
                         |   Cachet Server  |
                         |                  |
                         |  TCP Socket      |
                         |  HTTP Parser     |
                         |  Cache Layer     |
                         +--------+---------+
                                  |
                         +--------+--------+
                         |                 |
                       HIT               MISS
                         |                 |
                         v                 v
                 +---------------+  +---------------+
                 | Cached        |  | Remote Server |
                 | Response      |  | Connection    |
                 +-------+-------+  +-------+-------+
                         |                  |
                         |                  v
                         |           Receive Response
                         |                  |
                         |                  v
                         |            Store in Cache
                         |                  |
                         +--------+---------+
                                  |
                                  v
                         +------------------+
                         | Response to      |
                         | Client           |
                         +------------------+
```


## Request Flow

1. Accept a TCP client connection.
2. Receive the HTTP request.
3. Parse the method, path, and `Host` header.
4. Generate a cache key using `host + path`.
5. Check the in-memory cache.
6. On a HIT, send the cached response.
7. On a MISS, connect to the remote server and forward the request.
8. Receive the remote response.
9. Store the response in the cache.
10. Send the response to the client.

A cache hit avoids contacting the remote server.

## HTTP Request Handling

Cachet currently focuses on HTTP GET requests.

Example request:

```text
GET /1 HTTP/1.1
Host: example.com
```

The generated cache key is:

```text
example.com/1
```

Cachet receives the request over TCP and detects the end of the HTTP headers before parsing the request.

## Cache System

Cachet uses a fixed-size in-memory cache.

Each cache entry contains:

```text
CacheEntry
├── key
├── response
├── response_size
└── last_used
```

- `key` identifies the requested resource.
- `response` stores the remote response.
- `response_size` stores its size.
- `last_used` is used for LRU eviction.

### Cache MISS

```text
MISS
 |
 v
Remote Server
 |
 v
Receive Response
 |
 v
Store in Cache
 |
 v
Send to Client
```

### Cache HIT

```text
HIT
 |
 v
Retrieve Cached Response
 |
 v
Send to Client
```

## Thread Safety

Cachet creates a separate POSIX thread for each connected client.

The cache is shared between threads, so cache operations are protected using a POSIX mutex.

The mutex protects:

- Cache lookup
- Cache insertion
- LRU metadata updates
- Cache replacement
- Access counter updates

```text
Acquire Mutex
     |
Access / Modify Cache
     |
Release Mutex
```

## LRU Cache Eviction

Cachet uses a Least Recently Used (LRU) replacement strategy.

Each cache entry has a `last_used` value. A global access counter is updated when entries are inserted or accessed.

When the cache is full, Cachet finds the entry with the smallest `last_used` value and replaces it.

Example:

```text
/1 -> 15
/2 -> 18
/3 -> 12
/4 -> 21
```

Here `/3` is the least recently used entry.

## Memory Management

Cachet dynamically allocates memory for:

- Cache keys
- Cached responses
- Per-client socket descriptors

When an LRU entry is replaced, its allocated key and response memory are released before reuse.

During a cache hit, the cached response is copied into newly allocated memory before being sent to the client.

## Multithreaded Client Handling

The main server continuously accepts connections and creates a detached thread for each client.

```text
accept()
   |
   v
pthread_create()
   |
   v
handle_client()
   |
   v
Process Request
   |
   v
Send Response
```

Detached threads allow completed thread resources to be reclaimed without requiring the main thread to join them.

## Remote Server Communication

On a cache miss, Cachet:

1. Resolves the remote hostname.
2. Creates a TCP socket.
3. Connects to the remote server.
4. Forwards the HTTP request.
5. Receives the response.
6. Stores the response in the cache.
7. Sends the response to the client.

The remote communication layer is implemented separately from the main server and cache.

## Terminal Diagnostics

Cachet provides structured runtime output showing the request lifecycle.

Example:

```text
[Client] Connected.

========== NEW CLIENT REQUEST ==========

[HTTP] GET /1
[HTTP] Host: example.com

[Cache] Cache key: example.com/1
[Cache] Trying to lock cache...
[Cache] Cache lock acquired.
[Cache] Cache MISS.
[Cache] Releasing cache lock.

[Remote] Request forwarded to remote server.
[Remote] Response received from remote server.

[Cache] Response stored in cache.

[Client] Response sent to client.

========== REQUEST COMPLETE ==========
```

A repeated request produces:

```text
[Cache] Cache HIT.
[Client] Cached response sent to client.
```

This makes the internal request flow easy to observe without printing the complete remote response.

## Project Structure

```text
Cachet/
├── include/
│   ├── http.h
│   ├── remote.h
│   └── cache.h
├── src/
│   ├── cachet.c
│   ├── http.c
│   ├── remote.c
│   └── cache.c
├── .gitignore
├── LICENSE
├── Makefile
└── README.md
```

### Source Files

- **`src/cachet.c`** — TCP server, client acceptance, and multithreaded handling.
- **`src/http.c`** — HTTP request parsing.
- **`src/remote.c`** — Remote server connection and communication.
- **`src/cache.c`** — Cache lookup, insertion, mutex protection, and LRU eviction.

The `include/` directory contains the public interfaces for these modules.

## Build and Run

Build:

```bash
make
```

Run:

```bash
make run
```

Clean:

```bash
make clean
```

Rebuild:

```bash
make rebuild
```

The server listens on port `8080`.

## Testing

Start the server:

```bash
make run
```

In another terminal:

```bash
printf 'GET /1 HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n' | nc 127.0.0.1 8080
```

### Cache MISS

The first request should show:

```text
[Cache] Cache MISS.
[Remote] Request forwarded to remote server.
[Remote] Response received from remote server.
[Cache] Response added to cache.
```

### Cache HIT

Send the same request again without restarting the server:

```text
[Cache] Cache HIT.
[Client] Cached response sent to client.
```

The second request is served from the cache without contacting the remote server.

### LRU Testing

Fill the cache with different resources such as:

```text
/1 /2 /3 ... /10 /11
```

When the cache is full, the least recently used entry is evicted. Accessing an older entry before inserting a new resource changes which entry is selected for eviction.

## Design Decisions

### Modular Architecture

Networking, HTTP parsing, remote communication, and caching are separated into different modules. This keeps each component focused and easier to maintain.

### In-Memory Cache

An in-memory cache provides fast lookups without requiring an external database or persistent storage.

### Mutex-Protected Cache

A POSIX mutex protects shared cache data because multiple client threads can access it concurrently.

### LRU Replacement

LRU keeps recently accessed resources available while removing older unused entries when the cache is full.

### Host + Path Cache Key

Cache keys are created using:

```text
host + path
```

For example:

```text
example.com + /index.html
```

becomes:

```text
example.com/index.html
```

This prevents identical paths on different hosts from sharing the same cache entry.

### Detached Threads

Client threads are detached so the main server does not need to join completed threads.

## Current Limitations

- Supports the current HTTP GET-oriented request flow.
- Uses a fixed-size in-memory cache.
- Cached responses are limited by the current response buffer.
- Remote communication currently uses HTTP on port 80.
- HTTP parsing is intentionally lightweight.
- Cache contents are lost when the server terminates.
- Cache is not persisted to disk.
- HTTPS/TLS proxying is not currently supported.

## Future Improvements

Possible extensions include:

- HTTPS / CONNECT proxy support
- Configurable cache size
- Persistent disk-backed cache
- More complete HTTP parsing
- Additional HTTP methods
- Streaming large responses
- Configurable remote ports
- Better partial `send()` handling
- Cache statistics and hit-rate reporting
- Graceful server shutdown
- Configuration file support
- Automated integration tests

## What Cachet Demonstrates

```text
C Programming
   |
   +-- TCP Sockets
   +-- HTTP Parsing
   +-- Client/Server Communication
   +-- DNS Resolution
   +-- Multithreading
   +-- Mutex Synchronization
   +-- Dynamic Memory Management
   +-- In-Memory Caching
   +-- LRU Cache Eviction
   +-- Modular Software Design
```

Cachet demonstrates how low-level socket programming, concurrency, synchronization, memory management, and caching can be combined to build a working network service.

## License

This project is licensed under the MIT License.

## Project Status

Cachet is a completed working prototype of a multithreaded HTTP proxy server with an in-memory LRU cache.

The implementation covers TCP communication, HTTP request parsing, remote server communication, multithreaded client handling, thread-safe caching, cache hit/miss handling, LRU eviction, dynamic memory management, and modular C design.

## Author

Built as a systems programming project to explore networking, concurrency, caching, and memory management in C.
