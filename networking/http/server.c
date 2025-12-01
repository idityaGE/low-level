#include <arpa/inet.h>   // For inet_ntoa()
#include <netinet/in.h>  // For sockaddr_in, INADDR_ANY, htons()
#include <pthread.h>     // For pthread_create(), pthread_detach()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // For socket(), bind(), listen(), accept()
#include <unistd.h>      // For close()

#define PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 10  // Number of pending connections in the queue

void* handle_request(void* arg) {
  char buffer[BUFFER_SIZE] = {0};
  char message[BUFFER_SIZE] = "Hello from Server!";
  ssize_t valread;
  int new_socket = *((int*)arg);
  free(arg);  // Free the allocated memory for the socket descriptor

  valread = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
  if (valread == -1) {
    perror("recv failed");
  } else if (valread == 0) {
    printf("Client disconnected.\n");
  } else {
    buffer[valread] = '\0';  // Null-terminate the received data
    printf("Client message: %s\n", buffer);
    send(new_socket, message, strlen(message), 0);  // Echo back the message
  }
  close(new_socket);
  return NULL;
}

int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  // 1. Create socket file descriptor
  // AF_INET: IPv4
  // SOCK_STREAM: TCP
  // 0: Default protocol for the given family and type or For TCP, it's  IPPROTO_TCP; for UDP, IPPROTO_UDP
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Optional: Set socket options to reuse address and port immediately after closing
  // This helps avoid "Address already in use" errors if you restart the server quickly
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  // Prepare the sockaddr_in structure
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;  // Listen on all available network interfaces
  address.sin_port = htons(PORT);        // Convert port to network byte order

  // 2. Bind the socket to the specified IP and port
  if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 3. Listen for incoming connections
  if (listen(server_fd, BACKLOG) < 0) {
    perror("listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", PORT);

  // 4. Accept connections in a loop and spawn threads to handle them
  while (1) {
    // This call blocks until a client connects
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
      perror("accept failed");
      continue;  // Continue accepting other connections
    }
    printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    // Allocate memory for the socket descriptor to pass to the thread
    int* new_sock = malloc(sizeof(int));
    if (new_sock == NULL) {
      perror("malloc failed");
      close(new_socket);
      continue;
    }
    *new_sock = new_socket;

    // Create a new thread to handle the client request
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, handle_request, (void*)new_sock) != 0) {
      perror("pthread_create failed");
      free(new_sock);
      close(new_socket);
      continue;
    }

    // Detach the thread so it cleans up automatically when done
    pthread_detach(thread_id);
  }

  close(server_fd);  // Close the main listening socket

  return 0;
}