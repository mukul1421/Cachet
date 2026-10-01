# Cachet

Cachet is a multithreaded HTTP proxy server in C with an LRU cache — forwards client requests, caches responses for fast repeat lookups, and serves multiple clients at once.

## Project Status
🚧 Currently under development.

## Planned Features

* Bare-bones socket server (done)
* HTTP request parsing
* Request forwarding to origin server
* LRU cache with hit/miss logging
* Multithreaded client handling
* Thread-safe cache (mutex-protected)
* Path-traversal protection
* Benchmark: cache hit vs. cache miss timing

## License
This project is licensed under the MIT License.
