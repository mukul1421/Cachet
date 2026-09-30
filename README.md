Cachet: a Multithreaded HTTP Proxy with an LRU Cache

Cachet is a small HTTP proxy server written from scratch in C. It sits between a browser and the internet, forwards requests to the real web server, and keeps recently fetched pages in memory so repeat requests are answered instantly. Many browsers can use it at the same time.

Status

Update this list as you go, so the README always matches the code.

 Bare-bones socket server (accept connection, hardcoded response) — src/main.c
 Single-threaded proxy (parse request, forward, relay response)
 LRU cache with hit/miss logging
 Multithreading (one thread per connection, with a connection limit)
 Thread-safe cache (mutex), checked with ThreadSanitizer / Helgrind
 Benchmarks with and without the cache
 Stretch: thread pool, cache statistics, cache expiry, domain blocklist
1. Project Description

A proxy server is an intermediary. Instead of a browser talking directly to example.com, it asks the proxy, and the proxy fetches the page and passes it back.

Browser  ---->  Cachet Proxy  ---->  Web Server
                    |
                 LRU Cache

Cachet adds two features on top of plain forwarding:

Multithreading: several clients are served at the same time.
LRU cache: the responses to recent requests are stored in memory. When the cache is full, the Least Recently Used entry is removed first.
2. Goals
Learn socket programming, HTTP basics, and POSIX threads in C.
Implement a correct and efficient LRU cache (O(1) lookup, insert, and eviction).
Make shared data thread-safe and prove it with tooling, not just by running it once.
Measure the benefit of caching and report real numbers.
Keep the code small, readable, and free of memory leaks.
3. Specifications
Item	Decision
Language	C (C11), built with gcc
Platform	Linux (POSIX sockets and pthreads)
Protocol	HTTP/1.x, plain http:// only (no HTTPS / CONNECT)
Methods	GET only; other methods get 501 Not Implemented
Malformed request	400 Bad Request
Concurrency	One thread per connection, capped by a connection limit
Cache size	1 MB total, 100 KB maximum per object (configurable in include/config.h)
Cache key	Full request URL
Cache policy	LRU eviction
External libraries	None (C standard library, POSIX, pthreads)
Out of scope

HTTPS tunneling, POST and other methods, HTTP keep-alive, and honoring Cache-Control headers. These are listed as possible future work.

4. Design
4.1 Components
File	Responsibility
src/main.c	Read the port from the command line, set up the listening socket, accept loop
src/proxy.c	Handle one client: read the request, check the cache, contact the origin server, reply
src/http_parser.c	Parse the request line and headers into a struct
src/cache.c	LRU cache: lookup, insert, evict, stats
include/*.h	Public interfaces and configuration constants
4.2 Request flow
Accept a client connection and hand it to a new thread.
Read from the socket until the blank line (\r\n\r\n) that ends the headers.
Parse the method, host, port (default 80), and path. Reject bad requests.
Look the URL up in the cache.
Hit: send the stored response and log CACHE HIT.
Miss: connect to the origin server, send a cleaned-up request, relay the response to the client, and store it in the cache if it fits. Log CACHE MISS.
Close the connection and free all memory for that request.
4.3 LRU cache design
A hash table maps URL to cache entry, so lookup is O(1).
A doubly linked list orders entries from most to least recently used.
On a hit, the entry is moved to the front of the list.
On insert, if the total size would exceed the limit, entries are removed from the back of the list until the new one fits.
Every entry owns its data (malloc), and eviction frees it.
4.4 Concurrency design
The cache is shared between threads, so every operation on it is protected by a pthread_mutex_t.
The lock is held only while touching the cache, never while doing network I/O, so a slow origin server does not block other clients.
A semaphore limits the number of active connections.
Verified with -fsanitize=thread and Valgrind (Helgrind).
5. Build and Run
Prerequisites
Linux
gcc and make
curl (for testing)
Optional: valgrind
bash
sudo apt install build-essential curl valgrind
Build
bash
make
Run
bash
./proxy 8080
Try it

In another terminal:

bash
curl -x http://localhost:8080 http://example.com/
curl -x http://localhost:8080 http://example.com/   # second request should be a cache hit

The proxy terminal logs each request:

[MISS] http://example.com/
[HIT]  http://example.com/

Many sites redirect to HTTPS, which this proxy does not support. Use plain-HTTP sites for testing.

6. Testing

Add your real results here.

Functional: compare the proxy's output with a direct curl of the same page.
Cache: request the same URL twice and confirm the second is a hit.
Eviction: fill the cache past its limit and confirm the oldest entry is removed.
Concurrency: run several curl processes at once, and run under ThreadSanitizer.
Memory: run under Valgrind and confirm no leaks.
Benchmark

Record measured numbers here after you run them. Do not fill this in until you have real data.

Scenario	Result
Cache miss (average time)	TBD
Cache hit (average time)	TBD
7. Project Layout
cachet-proxy/
├── src/
├── include/
├── tests/
├── Makefile
├── README.md
├── LICENSE
└── .gitignore
8. References
Beej's Guide to Network Programming: https://beej.us/guide/bgnet/
Video tutorial: "Build your own Web server | Multithreaded Proxy Web Server in C" (https://www.youtube.com/watch?v=eTvSgOoc_BE), used as a learning reference.
Add any other tutorials or code you learn from here.
9. Known Limitations and Future Work
HTTPS via CONNECT tunneling
Thread pool instead of one thread per connection
Respecting Cache-Control and expiry times
A domain blocklist
A statistics page
License

MIT. See LICENSE.
