#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>    // For pthreads
#include <semaphore.h>  // For sem_t, useful for signaling available slots (optional here, but common)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 10           // Max number of pending connections in the listen queue
#define THREAD_POOL_SIZE 20  // Max number of worker threads

// --- Task Queue Definition ---
typedef struct client_socket_node {
  int client_sock;
  struct client_socket_node* next;
} client_socket_node_t;

client_socket_node_t* queue_head = NULL;
client_socket_node_t* queue_tail = NULL;
int queue_size = 0;

// Mutex and Condition Variable for thread pool synchronization
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

// --- Function Prototypes ---
void* worker_thread(void* arg);
void add_client_to_queue(int client_sock);
int get_client_from_queue();

// --- Main Server Function ---
int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  pthread_t thread_pool[THREAD_POOL_SIZE];
  int i;

  // Initialize worker threads
  for (i = 0; i < THREAD_POOL_SIZE; i++) {
    if (pthread_create(&thread_pool[i], NULL, worker_thread, NULL) != 0) {
      perror("Failed to create worker thread");
      exit(EXIT_FAILURE);
    }
    printf("Worker thread %d created.\n", i + 1);
  }

  // 1. Create socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }
  printf("Socket created successfully.\n");

  // Optional: Set socket options to reuse address and port immediately after closing
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  // Prepare the sockaddr_in structure
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // 2. Bind the socket
  if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
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

  // Main loop: Accept new connections and add them to the queue
  while (1) {
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
      perror("accept failed");
      continue;  // Continue listening for other connections
    }
    printf("Connection accepted from %s:%d (socket: %d)\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), new_socket);

    // Add the new client socket to the queue for a worker thread to handle
    add_client_to_queue(new_socket);
  }

  // This part is typically unreachable in a server's main loop,
  // as it runs indefinitely. However, if you had a shutdown mechanism,
  // you would join threads here.
  for (i = 0; i < THREAD_POOL_SIZE; i++) {
    pthread_join(thread_pool[i], NULL);
  }

  close(server_fd);
  printf("Server shut down.\n");
  return 0;
}

// --- Worker Thread Function ---
void* worker_thread(void* arg) {
  char buffer[BUFFER_SIZE];
  ssize_t valread;
  int client_sock;

  while (1) {
    client_sock = get_client_from_queue();  // Get a client socket from the queue

    // If client_sock is -1, it means there might be an issue (e.g., queue shutdown)
    // or a very quick shutdown, in our current simple loop it mostly means valid client.
    if (client_sock == -1) {
      // For a graceful shutdown, a special value could signal threads to exit.
      // For now, continue waiting for new tasks.
      continue;
    }

    printf("Worker thread %lu handling client socket %d\n", (unsigned long)pthread_self(), client_sock);

    // --- Handle client communication ---
    memset(buffer, 0, BUFFER_SIZE);  // Clear buffer for new message
    valread = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);

    if (valread == -1) {
      perror("recv failed");
    } else if (valread == 0) {
      printf("Client socket %d disconnected.\n", client_sock);
    } else {
      buffer[valread] = '\0';  // Null-terminate the received data
      printf("Client %d message: %s\n", client_sock, buffer);

      // Echo back the message
      if (send(client_sock, buffer, strlen(buffer), 0) == -1) {
        perror("send failed");
      } else {
        printf("Message echoed back to client %d.\n", client_sock);
      }
    }

    // Close the client socket after handling
    close(client_sock);
    printf("Client socket %d closed by worker thread %lu.\n", client_sock, (unsigned long)pthread_self());
  }

  return NULL;
}

// --- Queue Functions ---

// Add a new client socket to the tail of the queue
void add_client_to_queue(int client_sock) {
  client_socket_node_t* new_node = (client_socket_node_t*)malloc(sizeof(client_socket_node_t));
  if (new_node == NULL) {
    perror("Failed to allocate memory for queue node");
    close(client_sock);  // Close the client socket if we can't queue it
    return;
  }
  new_node->client_sock = client_sock;
  new_node->next = NULL;

  pthread_mutex_lock(&queue_mutex);  // Protect the queue

  if (queue_head == NULL) {  // Queue is empty
    queue_head = new_node;
    queue_tail = new_node;
  } else {
    queue_tail->next = new_node;
    queue_tail = new_node;
  }
  queue_size++;
  printf("Client socket %d added to queue. Queue size: %d\n", client_sock, queue_size);

  pthread_cond_signal(&queue_cond);    // Signal a waiting worker thread that a task is available
  pthread_mutex_unlock(&queue_mutex);  // Release the queue lock
}

// Get a client socket from the head of the queue
int get_client_from_queue() {
  int client_sock = -1;

  pthread_mutex_lock(&queue_mutex);  // Protect the queue

  // Wait while the queue is empty
  while (queue_head == NULL) {
    printf("Worker thread %lu waiting for tasks...\n", (unsigned long)pthread_self());
    pthread_cond_wait(&queue_cond, &queue_mutex);
    // Spurious wakeups are possible, so we re-check the condition (queue_head == NULL)
    // after waking up, which is why a while loop is preferred over if.
  }

  // If we've reached here, there's a task in the queue
  client_socket_node_t* temp = queue_head;
  client_sock = temp->client_sock;
  queue_head = queue_head->next;
  if (queue_head == NULL) {  // If the queue becomes empty
    queue_tail = NULL;
  }
  queue_size--;
  free(temp);  // Free the memory of the dequeued node

  printf("Worker thread %lu dequeued client socket %d. Queue size: %d\n", (unsigned long)pthread_self(), client_sock, queue_size);

  pthread_mutex_unlock(&queue_mutex);  // Release the queue lock
  return client_sock;
}