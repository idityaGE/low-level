Creating a server in C on a Linux system involves using a set of system calls (functions provided by the operating system) and data structures to handle network communication. This process is generally referred to as socket programming.

Here's a detailed breakdown of how to create a simple TCP server, along with explanations of the calls, structs, parameters, and features:

## 1. Core Concepts

Before diving into the code, let's understand some core concepts:

*   **Socket:** An endpoint for communication. Think of it like a phone jack where you plug in your phone to make calls.
*   **TCP (Transmission Control Protocol):** A connection-oriented, reliable protocol that ensures data is delivered in order and without errors. It's like sending a registered letter.
*   **IP Address:** A unique identifier for a device on a network.
*   **Port Number:** A logical address within a device that identifies a specific application or service.
*   **Client-Server Model:** The client initiates a connection to the server, and the server listens for and accepts connections.

## 2. Server Creation Steps

A typical TCP server in C follows these steps:

1.  **Create a socket:** Obtain a file descriptor for a new socket.
2.  **Bind the socket to an address and port:** Associate the socket with a specific IP address and port number on the server machine.
3.  **Listen for incoming connections:** Put the socket in a listening state, waiting for clients to connect.
4.  **Accept a connection:** When a client tries to connect, accept the connection, creating a new socket for communication with that specific client.
5.  **Communicate:** Send and receive data with the connected client.
6.  **Close sockets:** Release resources when communication is complete.

## 3. Key System Calls and Structs

Let's break down the important system calls and data structures involved:

### A. Data Structures

#### 1. `sockaddr_in` and `sockaddr` (Address Structures)

These structures are used to store network address information (IP address and port number).

*   **`struct sockaddr_in`:** This is a more convenient and commonly used structure for IPv4 addresses.

    ```c
    #include <netinet/in.h> // For sockaddr_in

    struct sockaddr_in {
        sa_family_t     sin_family; // Address family (e.g., AF_INET for IPv4)
        in_port_t       sin_port;   // Port number (in network byte order)
        struct in_addr  sin_addr;   // IP address (in network byte order)
        unsigned char   sin_zero[8]; // Padding to make it the same size as sockaddr
    };

    struct in_addr {
        in_addr_t s_addr; // 32-bit IPv4 address (in network byte order)
    };
    ```

    *   **`sin_family`:** Specifies the address family. For IPv4, this will be `AF_INET`.
    *   **`sin_port`:** The port number. You need to convert this from host byte order to network byte order using `htons()`.
    *   **`sin_addr`:** The IP address. For a server listening on all available interfaces, you'd use `INADDR_ANY`, also converted to network byte order using `htonl()`.
    *   **`sin_zero`:** A padding field that is typically set to zero.

*   **`struct sockaddr`:** This is a generic socket address structure used by many socket functions. You often cast `sockaddr_in` to `sockaddr` when passing it to functions.

    ```c
    #include <sys/socket.h> // For sockaddr

    struct sockaddr {
        sa_family_t sa_family; // Address family
        char        sa_data[14]; // Protocol-specific address data
    };
    ```

    *   **`sa_family`:** Same as `sin_family`.
    *   **`sa_data`:** A generic buffer to hold the actual address. This is why you usually work with `sockaddr_in` and cast it.

#### 2. `addrinfo` (for `getaddrinfo`)

While `sockaddr_in` is fundamental, for more robust server development, especially if you want to support both IPv4 and IPv6, the `getaddrinfo` function and its associated `addrinfo` struct are very useful.

```c
#include <netdb.h> // For addrinfo

struct addrinfo {
    int              ai_flags;      // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;     // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;   // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;   // 0 for any, IPPROTO_TCP, IPPROTO_UDP
    socklen_t        ai_addrlen;    // Size of ai_addr in bytes
    struct sockaddr *ai_addr;       // Pointer to a sockaddr structure
    char            *ai_canonname;  // Canonical host name
    struct addrinfo *ai_next;       // Pointer to next list item
};
```

*   **`ai_flags`:** Flags that modify the behavior of `getaddrinfo`. `AI_PASSIVE` is crucial for servers, indicating that the returned socket address will be used in a `bind()` call.
*   **`ai_family`:** The address family you want (e.g., `AF_INET` for IPv4, `AF_INET6` for IPv6, `AF_UNSPEC` for either).
*   **`ai_socktype`:** The type of socket (e.g., `SOCK_STREAM` for TCP, `SOCK_DGRAM` for UDP).
*   **`ai_protocol`:** The protocol to use (e.g., `IPPROTO_TCP`, `IPPROTO_UDP`, or 0 for any).
*   **`ai_addrlen`:** Length of the `sockaddr` structure pointed to by `ai_addr`.
*   **`ai_addr`:** A pointer to a `sockaddr` structure (which could be `sockaddr_in` or `sockaddr_in6`).
*   **`ai_next`:** `getaddrinfo` can return a linked list of `addrinfo` structures, allowing you to try different configurations.

### B. System Calls

#### 1. `socket()`

*   **Purpose:** Creates a new communication endpoint (a socket) and returns a file descriptor for it.
*   **Syntax:**
    ```c
    int socket(int domain, int type, int protocol);
    ```
*   **Parameters:**
    *   **`domain` (or `family`):** Specifies the protocol family to be used.
        *   `AF_INET`: IPv4 Internet protocols.
        *   `AF_INET6`: IPv6 Internet protocols.
        *   `AF_UNIX` (or `AF_LOCAL`): Local communication within the same system.
    *   **`type`:** Specifies the type of socket.
        *   `SOCK_STREAM`: Provides sequenced, reliable, two-way, connection-based byte streams (TCP).
        *   `SOCK_DGRAM`: Supports datagrams (connectionless, unreliable messages of a fixed maximum length) (UDP).
        *   `SOCK_RAW`: Provides raw network protocol access.
    *   **`protocol`:** Specifies a particular protocol to be used with the socket. Usually set to `0` to let the system choose the appropriate protocol based on `domain` and `type`. For TCP, it's typically `IPPROTO_TCP`; for UDP, `IPPROTO_UDP`.
*   **Return Value:** On success, a non-negative file descriptor is returned. On error, `-1` is returned, and `errno` is set.

#### 2. `bind()`

*   **Purpose:** Assigns a local address (IP address and port number) to a socket. This is how the server "claims" a specific port.
*   **Syntax:**
    ```c
    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    ```
*   **Parameters:**
    *   **`sockfd`:** The file descriptor of the socket created by `socket()`.
    *   **`addr`:** A pointer to a `sockaddr` structure containing the local IP address and port number to bind to. You'll typically cast a `sockaddr_in` (or `sockaddr_in6`) structure to `sockaddr *`.
    *   **`addrlen`:** The size of the `addr` structure in bytes.
*   **Return Value:** On success, `0` is returned. On error, `-1` is returned, and `errno` is set.
*   **Features/Considerations:**
    *   If you bind to `INADDR_ANY` (for IPv4) or `::` (for IPv6) for the IP address, the server will listen on all available network interfaces.
    *   Binding to a specific IP address means the server will only accept connections on that interface.
    *   If the port is already in use (`EADDRINUSE`), `bind()` will fail.

#### 3. `listen()`

*   **Purpose:** Puts the socket into a passive listening mode, waiting for incoming client connection requests.
*   **Syntax:**
    ```c
    int listen(int sockfd, int backlog);
    ```
*   **Parameters:**
    *   **`sockfd`:** The file descriptor of the bound socket.
    *   **`backlog`:** The maximum number of pending connections that can be queued. If a client attempts to connect when the queue is full, the connection may be refused or dropped. A common value is `5` or `10`, but you can use `SOMAXCONN` for the system's maximum.
*   **Return Value:** On success, `0` is returned. On error, `-1` is returned, and `errno` is set.

#### 4. `accept()`

*   **Purpose:** Extracts the first connection request on the queue of pending connections for the listening socket, creates a new connected socket, and returns a new file descriptor for that socket. This new socket is then used for actual data exchange with the client.
*   **Syntax:**
    ```c
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    ```
*   **Parameters:**
    *   **`sockfd`:** The file descriptor of the listening socket.
    *   **`addr`:** A pointer to a `sockaddr` structure that will be filled with the client's address information upon successful connection. You'll typically pass a `sockaddr_in` (or `sockaddr_in6`) structure and cast it.
    *   **`addrlen`:** A pointer to a `socklen_t` variable that initially holds the size of the `addr` structure. Upon return, it will contain the actual size of the client's address.
*   **Return Value:** On success, a non-negative file descriptor for the *newly accepted* connection is returned. On error, `-1` is returned, and `errno` is set.
*   **Features/Considerations:**
    *   `accept()` is a **blocking** call by default. It will pause the program until a client connects.
    *   The original listening socket remains open and can continue to accept new connections. This is crucial for handling multiple clients.

#### 5. `send()` and `recv()` (or `read()` and `write()`)

*   **Purpose:** Used for sending and receiving data over a connected socket. Since sockets are essentially file descriptors, you can often use `read()` and `write()`, but `send()` and `recv()` offer more control (e.g., flags).
*   **`send()`:**
    ```c
    ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    ```
    *   **`sockfd`:** The connected socket file descriptor.
    *   **`buf`:** A pointer to the data to be sent.
    *   **`len`:** The number of bytes to send.
    *   **`flags`:** Optional flags (e.g., `MSG_DONTWAIT` for non-blocking send).
*   **`recv()`:**
    ```c
    ssize_t recv(int sockfd, void *buf, size_t len, int flags);
    ```
    *   **`sockfd`:** The connected socket file descriptor.
    *   **`buf`:** A pointer to the buffer where received data will be stored.
    *   **`len`:** The maximum number of bytes to receive.
    *   **`flags`:** Optional flags (e.g., `MSG_PEEK` to look at data without removing it from the buffer, `MSG_WAITALL` to wait until all `len` bytes are received or an error occurs).
*   **Return Value:** On success, the number of bytes sent/received is returned. On error, `-1` is returned, and `errno` is set. A return value of `0` for `recv()` usually indicates that the peer has closed its end of the connection.
*   **Features/Considerations:**
    *   TCP is a stream protocol, meaning data is treated as a continuous stream of bytes. There's no guarantee that a single `send()` call on one end will correspond to a single `recv()` call on the other end.
    *   Data can be sent/received in fragments. You might need to loop `send()` or `recv()` to ensure all desired data is processed.

#### 6. `close()`

*   **Purpose:** Closes a socket (file descriptor), releasing its resources.
*   **Syntax:**
    ```c
    int close(int fd);
    ```
*   **Parameters:**
    *   **`fd`:** The file descriptor of the socket to close.
*   **Return Value:** On success, `0` is returned. On error, `-1` is returned, and `errno` is set.
*   **Features/Considerations:**
    *   It's crucial to close both the listening socket (when the server shuts down) and the accepted client sockets (after communication with a client is finished).

#### 7. `htons()` and `htonl()` (Host to Network Short/Long)

*   **Purpose:** Convert values between host byte order and network byte order. Network byte order is always big-endian.
*   **`htons()`:** Converts a 16-bit unsigned integer (short) from host byte order to network byte order. Used for port numbers.
*   **`htonl()`:** Converts a 32-bit unsigned integer (long) from host byte order to network byte order. Used for IPv4 addresses.
*   **`ntohs()` and `ntohl()`:** Convert from network byte order to host byte order.

#### 8. `inet_pton()` and `inet_ntop()` (Presentation to Network/Network to Presentation)

*   **Purpose:** Convert IP addresses between human-readable text (presentation) and binary (network) formats.
*   **`inet_pton()`:** Converts an IP address from its text representation (e.g., "192.168.1.1") to its binary network format.
*   **`inet_ntop()`:** Converts an IP address from its binary network format to its text representation.

#### 9. `getaddrinfo()` (Advanced Address Resolution)

*   **Purpose:** A more modern and robust way to resolve hostnames and service names into socket addresses. It can handle both IPv4 and IPv6, making your code more future-proof.
*   **Syntax:**
    ```c
    int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP address
                    const char *service,  // e.g. "http" or "80"
                    const struct addrinfo *hints,
                    struct addrinfo **res);
    ```
*   **Parameters:**
    *   **`node`:** The hostname or IP address string. For a server, you'd usually pass `NULL` or a specific IP address.
    *   **`service`:** The service name (e.g., "http", "ftp") or port number string (e.g., "80", "21").
    *   **`hints`:** A pointer to an `addrinfo` structure that specifies criteria for selecting the socket addresses to be returned (e.g., `ai_family`, `ai_socktype`, `ai_flags`). For a server, `ai_flags` should include `AI_PASSIVE`.
    *   **`res`:** A pointer to a pointer to an `addrinfo` structure. `getaddrinfo` allocates a linked list of `addrinfo` structures and points `*res` to the first one.
*   **Return Value:** On success, `0` is returned. On error, a non-zero error code is returned.
*   **Features/Considerations:**
    *   You must call `freeaddrinfo(*res)` to free the memory allocated by `getaddrinfo()`.
    *   This function simplifies handling IPv4/IPv6 and allows you to specify whether you want `SOCK_STREAM` (TCP) or `SOCK_DGRAM` (UDP) sockets.

## 4. Example Server Code (Simple Echo Server)

This example demonstrates a basic TCP echo server that listens on port 12345, accepts one client, receives a message, and sends it back.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // For close()
#include <sys/socket.h> // For socket(), bind(), listen(), accept()
#include <netinet/in.h> // For sockaddr_in, INADDR_ANY, htons()
#include <arpa/inet.h>  // For inet_ntoa()

#define PORT 12345
#define BUFFER_SIZE 1024
#define BACKLOG 5 // Number of pending connections in the queue

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    ssize_t valread;

    // 1. Create socket file descriptor
    // AF_INET: IPv4
    // SOCK_STREAM: TCP
    // 0: Default protocol for the given family and type
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    // Optional: Set socket options to reuse address and port immediately after closing
    // This helps avoid "Address already in use" errors if you restart the server quickly
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all available network interfaces
    address.sin_port = htons(PORT);       // Convert port to network byte order

    // 2. Bind the socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Socket bound to port %d.\n", PORT);

    // 3. Listen for incoming connections
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Listening on port %d...\n", PORT);

    // 4. Accept a connection
    // This call blocks until a client connects
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    // 5. Communicate (receive and send back)
    valread = recv(new_socket, buffer, BUFFER_SIZE - 1, 0); // Read client message
    if (valread == -1) {
        perror("recv failed");
    } else if (valread == 0) {
        printf("Client disconnected.\n");
    } else {
        buffer[valread] = '\0'; // Null-terminate the received data
        printf("Client message: %s\n", buffer);
        send(new_socket, buffer, strlen(buffer), 0); // Echo back the message
        printf("Message echoed back to client.\n");
    }

    // 6. Close the connected socket and the listening socket
    close(new_socket);    // Close the client's connection socket
    close(server_fd);     // Close the main listening socket

    printf("Server shut down.\n");
    return 0;
}
```

## 5. Compiling and Running

1.  **Save:** Save the code as `server.c`.
2.  **Compile:** Use a C compiler (like GCC) on your Linux system:
    ```bash
    gcc server.c -o server
    ```
3.  **Run:** Execute the compiled program:
    ```bash
    ./server
    ```

To test this, you'd need a client. You can use `netcat` (a command-line tool) or write a simple C client.

**Using netcat as a client:**
Open another terminal and run:
```bash
nc localhost 12345
```
Then type a message and press Enter. The server should echo it back.

## 6. Advanced Server Features and Considerations

The example above is a single-client, blocking server. Real-world servers need to handle multiple clients concurrently and often employ more sophisticated techniques.

### A. Handling Multiple Clients

*   **`fork()` (Multi-process Server):**
    *   After `accept()`, the parent process calls `fork()` to create a new child process.
    *   The child process handles communication with the new client and then exits.
    *   The parent process continues to `accept()` new connections.
    *   **Pros:** Simpler to program, robust (crashes in one child don't affect others), uses OS-level isolation.
    *   **Cons:** Higher resource overhead per client (each `fork()` creates a new process with its own memory space), process creation can be slow.
*   **Pthreads (Multi-threaded Server):**
    *   After `accept()`, the parent process creates a new thread using `pthread_create()`.
    *   The new thread handles communication with the client and then terminates.
    *   The parent process continues to `accept()` new connections.
    *   **Pros:** Lower resource overhead than processes, faster context switching, threads share memory (easier data sharing).
    *   **Cons:** More complex to program (synchronization issues like race conditions, deadlocks), a crash in one thread can bring down the entire server.
*   **Non-blocking Sockets with `select()`, `poll()`, or `epoll()` (I/O Multiplexing):**
    *   Instead of blocking on `accept()` or `recv()`, these functions allow you to monitor multiple sockets (the listening socket and all connected client sockets) for events (e.g., data available to read, socket ready to write).
    *   You register the sockets you're interested in, and the OS tells you which ones are ready for I/O.
    *   **`select()`:** Oldest and most portable. Limits on number of file descriptors.
    *   **`poll()`:** More scalable than `select()` (no hard limit on file descriptors).
    *   **`epoll()`:** Linux-specific, highly scalable, and efficient for very large numbers of connections (e.g., thousands or millions). It uses an event-driven model.
    *   **Pros:** Highly efficient for many concurrent connections, single-process/single-thread architecture possible (event loop).
    *   **Cons:** More complex to implement correctly.

### B. Error Handling

*   Always check the return values of system calls.
*   Use `perror()` to print human-readable error messages based on `errno`.
*   Handle `SIGPIPE` if you are writing to a socket that the client has closed. By default, writing to a closed socket causes `SIGPIPE`, which terminates the process. You can either ignore `SIGPIPE` or handle it by checking `errno` for `EPIPE`.

### C. Graceful Shutdown

*   **`shutdown()`:** Allows you to partially close a socket (e.g., prevent further sends but still allow receives, or vice-versa) before fully closing it with `close()`. This can be useful for orderly connection termination.
*   Send a "farewell" message to clients before closing their connections.

### D. Security

*   **Input Validation:** Never trust client input. Validate all data received to prevent buffer overflows, injection attacks, etc.
*   **Authentication/Authorization:** For sensitive applications, implement mechanisms to verify client identity and permissions.
*   **Encryption (SSL/TLS):** Use libraries like OpenSSL to encrypt communication between client and server, protecting data privacy and integrity. This involves creating an SSL context, negotiating a handshake, and then using SSL-specific read/write functions.

### E. Configuration

*   Allow users to specify the port number or other parameters via command-line arguments or configuration files.

### F. Logging

*   Log server events, connections, disconnections, and errors to a file or standard output for debugging and monitoring.

### G. Daemonization

*   For production servers, you often want them to run as background processes (daemons) without a controlling terminal. This involves steps like forking, detaching from the terminal, changing the working directory, and closing standard I/O streams.

### H. `setsockopt()`

*   **Purpose:** Allows you to manipulate options for the socket.
*   **Syntax:** `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);`
*   **Parameters:**
    *   **`sockfd`:** The socket file descriptor.
    *   **`level`:** The protocol level at which the option resides (e.g., `SOL_SOCKET` for socket-level options, `IPPROTO_TCP` for TCP-specific options).
    *   **`optname`:** The option to set (e.g., `SO_REUSEADDR`, `SO_KEEPALIVE`, `TCP_NODELAY`).
    *   **`optval`:** A pointer to the value for the option.
    *   **`optlen`:** The size of the option value.
*   **Common Server-side Options:**
    *   `SO_REUSEADDR`: Allows the socket to bind to an address and port that are already in use, provided that the previous server process is in a `TIME_WAIT` state. Essential for quickly restarting servers.
    *   `SO_KEEPALIVE`: Enables periodic transmission of keep-alive messages on a connected socket. If no response is received, the connection is considered broken.
    *   `TCP_NODELAY` (at `IPPROTO_TCP` level): Disables the Nagle algorithm, which attempts to reduce the number of small packets by buffering data. Useful for low-latency applications but can increase network traffic.



================================================================================
================================================================================
================================================================================


### Client-side Steps

1.  **Create a socket:** Similar to the server, create a socket.
2.  **Connect to the server:** Establish a connection to the server's IP address and port.
3.  **Communicate:** Send data to the server and receive data from it.
4.  **Close socket:** Release resources.

### `client.c` Code

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // For close()
#include <sys/socket.h> // For socket(), connect()
#include <netinet/in.h> // For sockaddr_in, htons()
#include <arpa/inet.h>  // For inet_addr()

#define SERVER_IP "127.0.0.1" // Loopback address for localhost
#define SERVER_PORT 12345     // Must match the server's port
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    const char *message = "Hello from client!"; // Message to send

    // 1. Create socket file descriptor
    // AF_INET: IPv4
    // SOCK_STREAM: TCP
    // 0: Default protocol for the given family and type
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }
    printf("Client socket created.\n");

    // Prepare the sockaddr_in structure for the server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT); // Convert port to network byte order

    // Convert IPv4 and IPv6 addresses from text to binary form
    // inet_pton() is more modern and handles both IPv4 and IPv6
    // inet_addr() is older and only for IPv4
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 2. Connect to the server
    // This call blocks until a connection is established or an error occurs
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

    // 3. Communicate - Send message to server
    ssize_t bytes_sent = send(sock, message, strlen(message), 0);
    if (bytes_sent == -1) {
        perror("send failed");
    } else {
        printf("Message sent: \"%s\" (%zd bytes)\n", message, bytes_sent);
    }

    // 3. Communicate - Receive response from server
    ssize_t valread = recv(sock, buffer, BUFFER_SIZE - 1, 0); // Read server response
    if (valread == -1) {
        perror("recv failed");
    } else if (valread == 0) {
        printf("Server closed the connection.\n");
    } else {
        buffer[valread] = '\0'; // Null-terminate the received data
        printf("Server response: \"%s\" (%zd bytes)\n", buffer, valread);
    }

    // 4. Close the socket
    close(sock);
    printf("Client socket closed.\n");

    return 0;
}
```

### Explanation of Client-Specific Calls and Parameters

The client shares many of the same system calls and structs as the server, but the `connect()` function is central to the client's role.

#### 1. `connect()`

*   **Purpose:** Establishes a connection with a specified socket (the server's listening socket).
*   **Syntax:**
    ```c
    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    ```
*   **Parameters:**
    *   **`sockfd`:** The file descriptor of the client's socket (created with `socket()`).
    *   **`addr`:** A pointer to a `sockaddr` structure containing the **server's** IP address and port number to connect to. You'll typically cast a `sockaddr_in` (or `sockaddr_in6`) structure here.
    *   **`addrlen`:** The size of the `addr` structure in bytes.
*   **Return Value:** On success, `0` is returned. On error, `-1` is returned, and `errno` is set.
*   **Features/Considerations:**
    *   `connect()` is a **blocking** call by default. It will pause the program until the connection is established or an error occurs (e.g., server not listening, network unreachable, connection refused).
    *   If the connection is successful, the `sockfd` (client's socket) is now implicitly bound to an ephemeral (randomly chosen) local port and ready for `send()` and `recv()`.

#### 2. `inet_pton()`

*   **Purpose:** Converts an IP address from its text (presentation) form to its numerical binary form. It's more versatile than `inet_addr()` as it handles both IPv4 and IPv6.
*   **Syntax:**
    ```c
    int inet_pton(int af, const char *src, void *dst);
    ```
*   **Parameters:**
    *   **`af`:** The address family, typically `AF_INET` for IPv4 or `AF_INET6` for IPv6.
    *   **`src`:** A pointer to a null-terminated string containing the IP address in presentation format (e.g., "192.168.1.1" or "::1").
    *   **`dst`:** A pointer to a buffer where the numeric binary address will be stored. For `AF_INET`, this points to `struct in_addr.s_addr`.
*   **Return Value:**
    *   `1`: Success.
    *   `0`: `src` does not contain a valid address string for the specified family.
    *   `-1`: `af` is not a valid address family, and `errno` is set.
*   **Features/Considerations:**
    *   It's generally preferred over `inet_addr()` because it's safer (doesn't return `INADDR_NONE` on error, which is a valid IP address) and supports IPv6.

### Compiling and Running the Client

1.  **Save:** Save the code as `client.c`.
2.  **Compile:** Use a C compiler (like GCC) on your Linux system:
    ```bash
    gcc client.c -o client
    ```
3.  **Run:**
    *   First, make sure your `server` program is running in a separate terminal:
        ```bash
        ./server
        ```
    *   Then, in another terminal, run the client:
        ```bash
        ./client
        ```

You should see output similar to this:

**Server Output:**

```
Socket created successfully.
Socket bound to port 12345.
Listening on port 12345...
Connection accepted from 127.0.0.1:XXXXX  (XXXXX will be a random ephemeral port)
Client message: Hello from client!
Message echoed back to client.
Server shut down.
```

**Client Output:**

```
Client socket created.
Connected to server at 127.0.0.1:12345
Message sent: "Hello from client!" (18 bytes)
Server response: "Hello from client!" (18 bytes)
Client socket closed.
```

This simple client demonstrates the fundamental steps of establishing a TCP connection, sending data, and receiving a response. Just like the server, a production-ready client would require more robust error handling, possibly non-blocking I/O for interactive applications, and proper handling of disconnections.