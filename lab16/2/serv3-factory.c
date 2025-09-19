#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define QUEUE_SIZE 10
#define WORKER_THREADS 3
#define BACKLOG 5

struct clientRequest {
  int clientFd;
  struct sockaddr_in client_addr;
};

struct clientRequest requestQueue[QUEUE_SIZE];
int queue_front = 0, queue_rear = 0;
int queue_count = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t queue_empty_sem;
sem_t queue_full_sem;

void handleClient(int clientFd, struct sockaddr_in* client_addr) {
  char buffer[BUFFER_SIZE];
  char timeStr[50];
  time_t curTime;
  ssize_t bytesReceived;

  bytesReceived = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);
  if (bytesReceived <= 0) {
    close(clientFd);
    return;
  }

  buffer[bytesReceived] = '\0';
  printf(
      "Thread %lu: Received from %s:%d: %s\n",
      pthread_self(),
      inet_ntoa(client_addr->sin_addr),
      ntohs(client_addr->sin_port),
      buffer);

  if (strcmp(buffer, "time") == 0) {
    curTime = time(NULL);
    ctime_r(&curTime, timeStr);
    send(clientFd, timeStr, strlen(timeStr), 0);
    printf("Thread %lu: Sent time to client\n", pthread_self());
  }

  close(clientFd);
}

void* threadWork(void* arg) {
  while (1) {
    struct clientRequest request;

    sem_wait(&queue_full_sem);
    pthread_mutex_lock(&queue_mutex);

    request = requestQueue[queue_front];
    queue_front = (queue_front + 1) % QUEUE_SIZE;
    queue_count--;

    pthread_mutex_unlock(&queue_mutex);
    sem_post(&queue_empty_sem);

    handleClient(request.clientFd, &request.client_addr);
  }
}

int main() {
  int serverFd, clientFd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t clientLen = sizeof(client_addr);
  pthread_t workers[WORKER_THREADS];
  int i;

  sem_init(&queue_empty_sem, 0, QUEUE_SIZE);
  sem_init(&queue_full_sem, 0, 0);

  for (i = 0; i < WORKER_THREADS; i++) {
    pthread_create(&workers[i], NULL, threadWork, NULL);
  }

  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd == -1) {
    perror("socket creation failed");
    return -1;
  }

  int reuse = 1;
  setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(serverFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("bind failed");
    close(serverFd);
    return -1;
  }

  if (listen(serverFd, BACKLOG) == -1) {
    perror("listen failed");
    close(serverFd);
    return -1;
  }

  printf("Producer-Consumer server listening on port %d\n", PORT);

  while (1) {
    clientFd = accept(serverFd, (struct sockaddr*)&client_addr, &clientLen);
    if (clientFd == -1) {
      perror("accept failed");
      continue;
    }

    sem_wait(&queue_empty_sem);
    pthread_mutex_lock(&queue_mutex);

    requestQueue[queue_rear].clientFd = clientFd;
    requestQueue[queue_rear].client_addr = client_addr;
    queue_rear = (queue_rear + 1) % QUEUE_SIZE;
    queue_count++;

    pthread_mutex_unlock(&queue_mutex);
    sem_post(&queue_full_sem);

    printf("Added client to queue. Queue size: %d\n", queue_count);
  }

  close(serverFd);
  for (i = 0; i < WORKER_THREADS; i++) {
    pthread_cancel(workers[i]);
    pthread_join(workers[i], NULL);
  }

  sem_destroy(&queue_empty_sem);
  sem_destroy(&queue_full_sem);
  return 0;
}