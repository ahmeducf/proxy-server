# Proxy Server

Proxy is a simple concurrent HTTP proxy server that caches recently-requested web objects.

It's an application for learning about ***Socket Programming*** and ***Concurrent Programming***.


## Table of contents

|                                        Table of contents                                       |
|:----------------------------------------------------------------------------------------------:|
|                                      [Overview](#Overview)                                     |
|                          [How does proxy work?](#How-does-proxy-work)                          |
| [How does proxy deal with concurrent requests?](#How-does-proxy-deal-with-concurrent-requests) |
| [How does proxy synchronize accesses to cache?](#How-does-proxy-synchronize-accesses-to-cache) |
|                                  [Requirements](#Requirements)                                 |
|                            [How to test proxy?](#How-to-test-proxy)                            |
|                           [My thoughts about the project](#Thoughts)                           |








## Overview

A web proxy is a program that acts as a middleman between a Web browser and an end server.
- Instead of contacting the end server directly to get a Web page, the browser contacts the proxy, which forwards the request on to the end server. When the end server replies to the proxy, the proxy sends the reply on to the browser.
- It can deal with multiple concurrent connections using multi-threading.
- It can also cache web objects by storing local copies of objects from servers then responding to future requests by reading them out of its cache rather than by communicating again with remote servers.


## How does proxy work?

It has the following detailed behavior:
- **Set up** the proxy to accept incoming connections:

    1. Proxy creates a listening descriptor that is ready to receive connection requests on port ***`port`*** by calling ***`Open_listenfd()`*** funciton defined in **`socket.c`** file.

- **Set up** the proxy to deal with multiple concurrent connections using **prethreading** technique:

    1. After initializing buffer ***`sbuf`*** declared in **`sbuf.h`** file, the main thread creates the set of worker threads by calling ***`Pthread_create()`*** defined in **`thread.c`** file.
    2. Main thread then enters an infinite loop, waiting for connection requests using ***`Accept()`*** function defined in **`socket.c`** file.
    3. It then inserts the resulting connected descriptors in ***`sbuf`***.
    4. Each worker thread waits until it is able to remove a connected descriptor from the buffer and then calls the ***`serve_client()`*** function to serve the client.
   
- **Run** ***`serve_client()`*** routine:

    1. Read and parse the _HTTP_ request sent from the client by calling ***`read_HTTP_request()`*** function defined in **`serve.c`** file.
    2. using ***`hash()`*** function defined in **`cache.c`** file, generate ***`HTTP_request_hash`*** that will be used to check if the cache contains the requested web object.
    3. If the object is cached then read the object out of the cache rather than communicating again with the server by calling ***`service_from_cache()`*** function defined in **`service.c`** file.
    4. Otherwise, using ***`service_from_server()`*** function, Try to connect the server then send it the ***`parsed_request`***, read its response, write it back to the client, and save it in an internal buffer for possible caching.
    5. If ***`object_size`*** is less than ***`MAX_OBJECT_SIZE`*** write the object in a suitable cahce line.


## How does proxy deal with concurrent requests?

- We incur non-trivial cost of creating a new thread for each new client.
- A proxy based on *prethreading* tries to reduce this overhead by using the **producer-consumer** model shown in the figure.
- The proxy consists of a ***main thread*** and a set of ***worker threads***, the main thread repeatedly accepts connection requests from clients and places the resulting connected descriptors in a bounded buffer.
- Each worker thread repeatedly removes a descriptor from the buffer, services the client, and then waits for the next descriptor.

<p align="center"><img src="https://i.ibb.co/jfNwR5n/producer-consumer-model.png"></p>


## How does proxy synchronize accesses to cache?

- Accesses to the cache must be thread-safe, and free of race conditions, So as a matter of fact we have these special requirements:
    1. Multiple threads must be able to simultaneously read from the cache.
    2. No thread can write to specific object while another thread is reading it.
    3. Only one thread should be permitted to write to the object at a time, but that restriction mustn't exist for readers.

- As such, protecting accesses to the cache with *one large exclusive lock* was not an acceptable solution. ***We partioned the cache into lines***, each line associated with ***read_cnt*** that count the number of current readers of that cache line, and ***mutex_writing_cache_line*** semaphore that ***lock only the cache line associated with it*** instead of locking the whole cache.

- A **writer thread** locks the write mutex each time it writes to the cache line associtaed with it, and unlocks it each time it finishes writing. This guarantees that there is at most one writer in that cache line at any point of time.

- On the other hand, only the first **reader thread** to read a cache line locks write mutex for that cache line, and only the last reader to finish reading unlocks it. The write mutex is ignored by readers who enter and leave while other readers are present.
- This means that as long as a single reader holds the write mutex for a particular cahce line, an unbounded number of readers can read this cache line at the same time unimpeded.



## Requirements
- Git
- GCC
- make

## How to test proxy?
1) **Compile source code and run proxy**

    Run the following commands in a terminal:
    ```console
    git clone https://github.com/ahmed-salah-ucf/proxy-server.git
    ```
    ```console
    make
    ```
    ```console
    .\proxy <port>
    ```

2) **Send HTTP requests using** *telnet* **or** *curl* **tools to test the proxy**

    **telnet:**
    ```console
    telnet localhost <port>
    ```
    **curl:**
    ```console
    curl --proxy  http://localhost:<port> www.example.com
    ```

## Thoughts
- The Project helped me become more familiar with the concepts of **network programming** and **concurrent programming**.
- I learned about **HTTP operation** and how to use **sockets** to write programs that communicate over network connections.
- This introduced me to dealing with **concurrency**, and how to write **thread-safe** routines.
