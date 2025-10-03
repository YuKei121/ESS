#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_MSG_SIZE 256
#define SHM_KEY 0x12345
#define SEM_KEY 0x12346
#define HISTORY_SIZE 100

struct message {
  char text[MAX_MSG_SIZE];
  pid_t pid;
};

struct sharedData {
  int messageCount;
  struct message messages[HISTORY_SIZE];
};

WINDOW *chatWin, *inputWin;
int shm;
int sem;
struct sharedData* shmData;
pid_t clientPid;
char nickname[50];

struct sembuf lockOp = {0, -1, 0};
struct sembuf unlockOp = {0, 1, 0};

void lockSem() {
  semop(sem, &lockOp, 1);
}

void unlockSem() {
  semop(sem, &unlockOp, 1);
}

void initNcurses() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  refresh();

  int height, width;
  getmaxyx(stdscr, height, width);

  chatWin = newwin(height - 4, width, 1, 0);
  scrollok(chatWin, TRUE);
  box(chatWin, 0, 0);
  mvwprintw(chatWin, 0, 2, " Chat ");

  inputWin = newwin(3, width, height - 3, 0);
  box(inputWin, 0, 0);
  mvwprintw(inputWin, 0, 2, " Input ");

  wmove(chatWin, 1, 1);
  wmove(inputWin, 1, 1);

  wrefresh(chatWin);
  wrefresh(inputWin);
}

void* updateChat(void* arg) {
  int lastMessage = 0;

  while (1) {
    lockSem();
    while (lastMessage < shmData->messageCount) {
      int idx = lastMessage % HISTORY_SIZE;
      const char* msg = shmData->messages[idx].text;

      int y, x;
      getyx(chatWin, y, x);

      if (y >= getmaxy(chatWin) - 2) {
        scroll(chatWin);
        y = getmaxy(chatWin) - 2;
      }

      wmove(chatWin, y, 1);
      wprintw(chatWin, "%-*s", getmaxx(chatWin) - 2, msg);
      wmove(chatWin, y + 1, 1);

      if (y >= getmaxy(chatWin) - 2) {
        touchwin(chatWin);
        box(chatWin, 0, 0);
        mvwprintw(chatWin, 0, 2, " Chat ");
      }

      lastMessage++;
    }

    wrefresh(chatWin);
    unlockSem();
    usleep(100000);
  }
}

void sendMessage(const char* text) {
  lockSem();

  int idx = shmData->messageCount % HISTORY_SIZE;

  char formedMsg[MAX_MSG_SIZE];
  int written =
      snprintf(formedMsg, sizeof(formedMsg), "[%s]: %s", nickname, text);
  if (written >= sizeof(formedMsg)) {
    formedMsg[sizeof(formedMsg) - 1] = '\0';
  }

  strncpy(shmData->messages[idx].text, formedMsg, MAX_MSG_SIZE - 1);
  shmData->messages[idx].pid = clientPid;
  shmData->messageCount++;

  unlockSem();
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <nickname>\n", argv[0]);
    return -1;
  }

  strncpy(nickname, argv[1], sizeof(nickname) - 1);
  clientPid = getpid();

  shm = shmget(SHM_KEY, sizeof(struct sharedData), 0666);
  if (shm < 0) {
    perror("shmget");
    return -1;
  }

  shmData = shmat(shm, NULL, 0);
  if (shmData == (void*)-1) {
    perror("shmat");
    return -1;
  }

  sem = semget(SEM_KEY, 1, 0666);
  if (sem < 0) {
    perror("semget");
    return -1;
  }

  initNcurses();
  wrefresh(chatWin);

  pthread_t thread;
  pthread_create(&thread, NULL, updateChat, NULL);

  char inputBuffer[MAX_MSG_SIZE] = {0};
  int ch, pos = 0;

  while ((ch = wgetch(inputWin)) != 27) {  // Esc to leave
    if (ch == '\n') {
      if (pos > 0) {
        inputBuffer[pos] = '\0';

        sendMessage(inputBuffer);
        pos = 0;

        werase(inputWin);
        box(inputWin, 0, 0);
        mvwprintw(inputWin, 0, 2, " Input ");
        wmove(inputWin, 1, 1);
        wrefresh(inputWin);
      }
    } else if (ch == KEY_BACKSPACE || ch == 127) {
      if (pos > 0) {
        pos--;
        int y, x;
        getyx(inputWin, y, x);
        mvwaddch(inputWin, y, x - 1, ' ');
        wmove(inputWin, y, x - 1);
        wrefresh(inputWin);
      }
    } else if (pos < MAX_MSG_SIZE - 1) {
      inputBuffer[pos++] = ch;
      wechochar(inputWin, ch);
    }
  }

  shmdt(shmData);
  endwin();
  return 0;
}