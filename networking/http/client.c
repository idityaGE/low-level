#include <arpa/inet.h>   // For inet_addr()
#include <netinet/in.h>  // For sockaddr_in, htons()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // For socket(), connect()
#include <unistd.h>      // For close()

#define SERVER_IP "127.0.0.1"  // Loopback address for localhost
#define SERVER_PORT 8080      // Must match the server's port
#define BUFFER_SIZE 1024

int main() {
  int sock = 0;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE] = {0};
  const char* message = "Hello from client!";  // Message to send

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
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);  // Convert port to network byte order

  // Convert IPv4 and IPv6 addresses from text to binary form
  // inet_pton() is more modern and handles both IPv4 and IPv6
  // inet_addr() is older and only for IPv4
  if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
    perror("Invalid address/ Address not supported");
    close(sock);
    exit(EXIT_FAILURE);
  }

  // 2. Connect to the server
  // This call blocks until a connection is established or an error occurs
  if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
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
  ssize_t valread = recv(sock, buffer, BUFFER_SIZE - 1, 0);  // Read server response
  if (valread == -1) {
    perror("recv failed");
  } else if (valread == 0) {
    printf("Server closed the connection.\n");
  } else {
    buffer[valread] = '\0';  // Null-terminate the received data
    printf("Server response: \"%s\" (%zd bytes)\n", buffer, valread);
  }

  // 4. Close the socket
  close(sock);
  printf("Client socket closed.\n");

  return 0;
}