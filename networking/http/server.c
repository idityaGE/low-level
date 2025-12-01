#include <arpa/inet.h>   // For inet_ntoa()
#include <netinet/in.h>  // For sockaddr_in, INADDR_ANY, htons()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // For socket(), bind(), listen(), accept()
#include <unistd.h>      // For close()

#define PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 5  // Number of pending connections in the queue

int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};
  ssize_t valread;
  char message[BUFFER_SIZE] = "Hello from Server!";

  // 1. Create socket file descriptor
  // AF_INET: IPv4
  // SOCK_STREAM: TCP
  // 0: Default protocol for the given family and type or For TCP, it's  IPPROTO_TCP; for UDP, IPPROTO_UDP
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket failed");
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

  // 4. Accept a connection
  // This call blocks until a client connects
  if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
    perror("accept failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }
  printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

  // 5. Communicate (receive and send back)
  valread = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);  // Read client message
  if (valread == -1) {
    perror("recv failed");
  } else if (valread == 0) {
    printf("Client disconnected.\n");
  } else {
    buffer[valread] = '\0';  // Null-terminate the received data
    printf("Client message: %s\n", buffer);
    send(new_socket, message, strlen(message), 0);  // Echo back the message
  }

  // 6. Close the connected socket and the listening socket
  close(new_socket);  // Close the client's connection socket
  close(server_fd);   // Close the main listening socket

  return 0;
}