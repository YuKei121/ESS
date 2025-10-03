#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_DRIVERS 10
#define PIPE_READ 0
#define PIPE_WRITE 1
#define BUFFER_SIZE 256

struct driver {
  pid_t pid;
  int status;  // 0 = Available; > 0 = time
  int userToDriver[2];
  int driverToUser[2];
};

struct driver drivers[MAX_DRIVERS];
int driversTotal = 0;

void handleSignal(int sig) {
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

void create_driver() {
  if (driversTotal >= MAX_DRIVERS) {
    printf("Too many drivers\n");
    return;
  }

  int userToDriver[2];
  int driverToUser[2];

  if (pipe(userToDriver) == -1 || pipe(driverToUser) == -1) {
    perror("pipe");
    return;
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    close(userToDriver[0]);
    close(userToDriver[1]);
    close(driverToUser[0]);
    close(driverToUser[1]);
    return;
  }

  if (pid == 0) {
    // Driver process
    close(userToDriver[PIPE_WRITE]);
    close(driverToUser[PIPE_READ]);

    int taskTimer = 0;
    time_t taskEnd = 0;
    fd_set fds;
    char buf[BUFFER_SIZE];

    while (1) {
      FD_ZERO(&fds);
      FD_SET(userToDriver[PIPE_READ], &fds);

      struct timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      int ret = select(userToDriver[PIPE_READ] + 1, &fds, NULL, NULL, &timeout);

      if (ret == -1) {
        perror("select");
        break;
      }

      // task completed?
      if (taskTimer > 0) {
        time_t now = time(NULL);
        if (now >= taskEnd) {
          taskTimer = 0;
          printf("Driver %d: Task completed\n", getpid());
        }
      }

      if (FD_ISSET(userToDriver[PIPE_READ], &fds)) {
        int n = read(userToDriver[PIPE_READ], buf, sizeof(buf));
        if (n <= 0) {
          break;
        }

        if (strncmp(buf, "SEND", 4) == 0) {
          int newTimer = atoi(buf + 5);
          if (taskTimer > 0) {
            time_t now = time(NULL);
            int remaining = taskEnd - now;
            if (remaining < 0) {
              remaining = 0;
            }
            char reply[BUFFER_SIZE];
            snprintf(reply, sizeof(reply), "Busy %d", remaining);
            write(driverToUser[PIPE_WRITE], reply, strlen(reply) + 1);
            printf("Driver %d: Busy %d\n", getpid(), remaining);
          } else {
            taskTimer = newTimer;
            taskEnd = time(NULL) + newTimer;
            write(driverToUser[PIPE_WRITE], "OK", 3);
            printf("Driver %d: Task %d\n", getpid(), newTimer);
          }
        } else if (strncmp(buf, "STATUS", 6) == 0) {
          char reply[BUFFER_SIZE];
          if (taskTimer > 0) {
            time_t now = time(NULL);
            int remaining = taskEnd - now;
            if (remaining < 0) {
              remaining = 0;
            }
            snprintf(reply, sizeof(reply), "Busy %d", remaining);
          } else {
            snprintf(reply, sizeof(reply), "Available");
          }
          write(driverToUser[PIPE_WRITE], reply, strlen(reply) + 1);
        }
      }
    }
    close(userToDriver[PIPE_READ]);
    close(driverToUser[PIPE_WRITE]);
    exit(0);
  } else {
    // user process
    close(userToDriver[PIPE_READ]);
    close(driverToUser[PIPE_WRITE]);

    drivers[driversTotal].pid = pid;
    drivers[driversTotal].status = 0;
    drivers[driversTotal].userToDriver[PIPE_WRITE] = userToDriver[PIPE_WRITE];
    drivers[driversTotal].driverToUser[PIPE_READ] = driverToUser[PIPE_READ];
    driversTotal++;
    printf("Driver created with PID: %d\n", pid);
  }
}

void send_task(pid_t pid, int taskTimer) {
  for (int i = 0; i < driversTotal; i++) {
    if (drivers[i].pid == pid) {
      char command[BUFFER_SIZE];
      snprintf(command, sizeof(command), "SEND %d", taskTimer);
      write(drivers[i].userToDriver[PIPE_WRITE], command, strlen(command) + 1);

      char reply[BUFFER_SIZE];
      fd_set fds;
      struct timeval timeout = {2, 0};

      FD_ZERO(&fds);
      FD_SET(drivers[i].driverToUser[PIPE_READ], &fds);

      int ret = select(
          drivers[i].driverToUser[PIPE_READ] + 1, &fds, NULL, NULL, &timeout);
      if (ret > 0) {
        int n = read(drivers[i].driverToUser[PIPE_READ], reply, sizeof(reply));
        if (n > 0) {
          printf("Driver %d reply: %s\n", pid, reply);
        }
      } else {
        perror("select");
      }
      return;
    }
  }
  printf("Driver with PID %d not found\n", pid);
}

void get_status(pid_t pid) {
  for (int i = 0; i < driversTotal; i++) {
    if (drivers[i].pid == pid) {
      write(drivers[i].userToDriver[PIPE_WRITE], "STATUS", 7);

      char reply[BUFFER_SIZE];
      fd_set fds;
      struct timeval timeout = {2, 0};

      FD_ZERO(&fds);
      FD_SET(drivers[i].driverToUser[PIPE_READ], &fds);

      int ret = select(
          drivers[i].driverToUser[PIPE_READ] + 1, &fds, NULL, NULL, &timeout);
      if (ret > 0) {
        int n = read(drivers[i].driverToUser[PIPE_READ], reply, sizeof(reply));
        if (n > 0) {
          printf("Driver %d status: %s\n", pid, reply);
        }
      } else {
        perror("select");
      }
      return;
    }
  }
  printf("Driver with PID %d not found\n", pid);
}

void get_drivers() {
  printf("Drivers (%d):\n", driversTotal);
  for (int i = 0; i < driversTotal; i++) {
    printf("Driver PID: %d - ", drivers[i].pid);

    write(drivers[i].userToDriver[PIPE_WRITE], "STATUS", 7);

    char reply[BUFFER_SIZE];
    fd_set fds;
    struct timeval timeout = {1, 0};

    FD_ZERO(&fds);
    FD_SET(drivers[i].driverToUser[PIPE_READ], &fds);

    int ret = select(
        drivers[i].driverToUser[PIPE_READ] + 1, &fds, NULL, NULL, &timeout);
    if (ret > 0) {
      int n = read(drivers[i].driverToUser[PIPE_READ], reply, sizeof(reply));
      if (n > 0) {
        printf("%s\n", reply);
      } else {
        printf("Unknown\n");
      }
    } else {
      printf("No reply\n");
    }
  }
}

int main() {
  signal(SIGCHLD, handleSignal);
  char command[BUFFER_SIZE];

  printf("Taxi CLI\n");
  printf(
      "Commands: create_driver, send_task <pid> <timer>, get_status <pid>, "
      "get_drivers, exit\n");

  while (1) {
    printf("> ");
    if (!fgets(command, sizeof(command), stdin)) {
      break;
    }

    command[strcspn(command, "\n")] = 0;

    if (strcmp(command, "create_driver") == 0) {
      create_driver();
    } else if (strncmp(command, "send_task", 9) == 0) {
      pid_t pid;
      int timer;
      if (sscanf(command + 10, "%d %d", &pid, &timer) == 2) {
        send_task(pid, timer);
      } else {
        printf("Usage: send_task <pid> <timer>\n");
      }
    } else if (strncmp(command, "get_status", 10) == 0) {
      pid_t pid;
      if (sscanf(command + 11, "%d", &pid) == 1) {
        get_status(pid);
      } else {
        printf("Usage: get_status <pid>\n");
      }
    } else if (strcmp(command, "get_drivers") == 0) {
      get_drivers();
    } else if (strcmp(command, "exit") == 0) {
      break;
    } else {
      printf("Unknown command\n");
    }
  }

  // Cleanup
  for (int i = 0; i < driversTotal; i++) {
    close(drivers[i].userToDriver[PIPE_WRITE]);
    close(drivers[i].driverToUser[PIPE_READ]);
    kill(drivers[i].pid, SIGTERM);
  }

  return 0;
}