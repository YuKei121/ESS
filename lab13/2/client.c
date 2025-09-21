#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_MSG_SIZE 256
#define SERVER_KEY 0x12345

struct message {
  long mtype;
  char mtext[MAX_MSG_SIZE];
  pid_t pid;
};

WINDOW *chatWin, *inputWin;
int msqid;
pid_t clientPid;

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

void* receiveMessages(void* arg) {
  struct message msg;
  while (1) {
    if (msgrcv(
            msqid, &msg, sizeof(struct message) - sizeof(long), clientPid, 0) <
        0) {
      perror("msgrcv");
      break;
    }

    int y, x;
    getyx(chatWin, y, x);

    if (y >= getmaxy(chatWin) - 2) {
      scroll(chatWin);
      y = getmaxy(chatWin) - 2;
    }

    wmove(chatWin, y, 1);
    wprintw(chatWin, "%-*s", getmaxx(chatWin) - 2, msg.mtext);
    wmove(chatWin, y + 1, 1);
    if (y >= getmaxy(chatWin) - 2) {
      touchwin(chatWin);
      box(chatWin, 0, 0);
      mvwprintw(chatWin, 0, 2, " Chat ");
    }
    wrefresh(chatWin);
  }
}

void sendMessage(long type, const char* text) {
  struct message msg = {type, "", clientPid};
  strncpy(msg.mtext, text, sizeof(msg.mtext) - 1);
  msg.mtext[sizeof(msg.mtext) - 1] = '\0';
  msgsnd(msqid, &msg, sizeof(struct message) - sizeof(long), 0);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <nickname>\n", argv[0]);
    return -1;
  }

  clientPid = getpid();

  if ((msqid = msgget(SERVER_KEY, 0666)) < 0) {
    perror("msgget");
    return -1;
  }

  sendMessage(1, argv[1]);

  initNcurses();
  wprintw(chatWin, "Connected to chat. Your PID: %d\n", clientPid);
  wrefresh(chatWin);

  pthread_t thread;
  pthread_create(&thread, NULL, receiveMessages, NULL);

  char inputBuffer[MAX_MSG_SIZE] = {0};
  int ch, pos = 0;

  // Esc to leave
  while ((ch = wgetch(inputWin)) != 27) {
    if (ch == '\n') {
      if (pos > 0) {
        inputBuffer[pos] = '\0';

        int y, x;
        getyx(chatWin, y, x);

        if (y >= getmaxy(chatWin) - 2) {
          scroll(chatWin);
          y = getmaxy(chatWin) - 2;
        }

        wmove(chatWin, y, 1);
        wprintw(chatWin, "%-*s", getmaxx(chatWin) - 2, inputBuffer);

        touchwin(chatWin);
        box(chatWin, 0, 0);
        mvwprintw(chatWin, 0, 2, " Chat ");

        wmove(chatWin, y + 1, 1);
        wrefresh(chatWin);

        sendMessage(2, inputBuffer);
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

  sendMessage(3, "exit");
  endwin();
  return 0;
}