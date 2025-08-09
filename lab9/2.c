#include <dirent.h>
#include <limits.h>
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#define MAX_ITEMS 1024
#define MAX_PATH 1024

void sig_winch(int signo) {
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char*)&size);
  resizeterm(size.ws_row, size.ws_col);
}

typedef struct {
  char* curPath;
  char* items[MAX_ITEMS];
  int itemCount;
  int curIndex;
  int topIndex;
  WINDOW* win;
} Panel;

Panel leftPanel, rightPanel;
Panel* activePanel;
int exitFlag = 0;

int fil(const struct dirent* entry) {
  return strcmp(entry->d_name, ".") != 0;
}

int compare(const struct dirent** a, const struct dirent** b) {
  const struct dirent* A = *a;
  const struct dirent* B = *b;

  if (strcmp(A->d_name, "..") == 0) {
    return -1;
  }
  if (strcmp(B->d_name, "..") == 0) {
    return 1;
  }

  int aIsDir = (A->d_type == DT_DIR);
  int bIsDir = (B->d_type == DT_DIR);

  if (aIsDir && !bIsDir) {
    return -1;
  }
  if (!aIsDir && bIsDir) {
    return 1;
  }

  return strcasecmp(A->d_name, B->d_name);
}

void updatePanel(Panel* panel) {
  struct dirent** namelist;
  int n = scandir(panel->curPath, &namelist, fil, compare);

  panel->itemCount = 0;
  for (int i = 0; i < n && panel->itemCount < MAX_ITEMS; i++) {
    panel->items[panel->itemCount++] = strdup(namelist[i]->d_name);
    free(namelist[i]);
  }
  free(namelist);

  if (panel->curIndex >= panel->itemCount) {
    panel->curIndex = panel->itemCount - 1;
  }
}

void initPanel(Panel* panel, const char* path, WINDOW* win) {
  panel->curPath = realpath(path, NULL);
  panel->win = win;
  panel->curIndex = 0;
  panel->topIndex = 0;
  updatePanel(panel);
}

void changeDirectory(Panel* panel, const char* name) {
  if (strcmp(name, "..") != 0) {
    char fullPath[MAX_PATH];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", panel->curPath, name);

    struct stat s;
    if (stat(fullPath, &s) == -1 || !S_ISDIR(s.st_mode)) {
      return;
    }
  }

  char newPath[MAX_PATH];
  snprintf(newPath, sizeof(newPath), "%s/%s", panel->curPath, name);

  char* realNewPath = realpath(newPath, NULL);
  if (realNewPath) {
    free(panel->curPath);
    panel->curPath = realNewPath;
    panel->curIndex = 0;
    updatePanel(panel);
  }
}

void draw_panel(Panel* panel, int is_active) {
  werase(panel->win);
  box(panel->win, 0, 0);

  int y, x;
  getmaxyx(panel->win, y, x);
  int height = y - 2;

  if (panel->curIndex < panel->topIndex) {
    panel->topIndex = panel->curIndex;
  } else if (panel->curIndex >= panel->topIndex + height) {
    panel->topIndex = panel->curIndex - height + 1;
  }

  for (int i = 0; i < height && i + panel->topIndex < panel->itemCount; i++) {
    int idx = i + panel->topIndex;
    int attr = (idx == panel->curIndex && is_active) ? A_REVERSE : A_NORMAL;
    wattron(panel->win, attr);
    mvwprintw(panel->win, i + 1, 1, "%.*s", x - 2, panel->items[idx]);
    wattroff(panel->win, attr);
  }

  wrefresh(panel->win);
}

int main() {
  initscr();
  signal(SIGWINCH, sig_winch);
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  int y, x;
  getmaxyx(stdscr, y, x);
  int width = x / 2;

  leftPanel.win = newwin(y, width, 0, 0);
  rightPanel.win = newwin(y, width, 0, width);

  initPanel(&leftPanel, ".", leftPanel.win);
  initPanel(&rightPanel, ".", rightPanel.win);
  activePanel = &leftPanel;

  refresh();

  while (!exitFlag) {
    draw_panel(&leftPanel, activePanel == &leftPanel);
    draw_panel(&rightPanel, activePanel == &rightPanel);

    int ch = getch();
    switch (ch) {
      case KEY_UP:
        if (activePanel->curIndex > 0)
          activePanel->curIndex--;
        break;
      case KEY_DOWN:
        if (activePanel->curIndex < activePanel->itemCount - 1)
          activePanel->curIndex++;
        break;
      case '\t':  // Tab
        activePanel = (activePanel == &leftPanel) ? &rightPanel : &leftPanel;
        break;
      case '\n':  // Enter
        if (activePanel->itemCount > 0) {
          char* name = activePanel->items[activePanel->curIndex];
          changeDirectory(activePanel, name);
        }
        break;
      case 27:  // ESC
        // maybe I should have used 'q' instead?
        // needs to be pressed twice
        exitFlag = 1;
        break;
    }
  }

  endwin();
  return 0;
}
