#include <stdio.h>
#include <string.h>
#define LIST_SIZE 100

void removeN(char* str) {
  while (*str != '\0') {
    if (*str == '\n') {
      *str = '\0';
      break;
    }
    str++;
  }
}

struct abonent {
  char name[10];
  char second_name[10];
  char tel[10];
};

struct abonent abonentList[LIST_SIZE];

void listInit() {
  for (int i = 0; i < LIST_SIZE; ++i) {
    struct abonent newAbonent = {{0}, {0}, {0}};
    // for testing purposes only: fills list with 100 nodes of gibberish
    // struct abonent newAbonent = {{49 + i}, {49 + i}, {49 + i}};
    abonentList[i] = newAbonent;
  }
}

int searchInList(int mode, char name[10]) {
  switch (mode) {
    case 0:
      printf("Searching for empty spot...\n");
      for (int i = 0; i < LIST_SIZE; ++i) {
        if (abonentList[i].name[0] == '\0') {
          return i;
        }
      }
      break;

    case 1:
      printf("Searching for %s...\n", name);
      for (int i = 0; i < LIST_SIZE; ++i) {
        if (strcmp(abonentList[i].name, name) == 0) {
          return i;
        }
      }
      break;

    default:
      printf("error has occured in searchInList: unknown mode\n");
      return -2;
  }
  return -1;
}

int main() {
  listInit();
  char userInput[3];
  char command = 0;
  while (command != '5') {
    printf(
        "\n"
        "1) Добавить абонента\n"
        "2) Удалить абонента\n"
        "3) Поиск абонентов по имени\n"
        "4) Вывод всех записей\n"
        "5) Выход\n");
    // scanf("%c", &userInput);
    fgets(userInput, sizeof(userInput), stdin);
    sscanf(userInput, "%c", &command);
    switch (command) {
      case '1':
        int newIndex = searchInList(0, NULL);
        if (newIndex < 0) {
          printf("Невозможно добавить абонента: список уже переполнен\n");
          break;
        }
        struct abonent newAbonent = {{0}, {0}, {0}};
        printf("Введите имя нового абонента: ");
        fgets(newAbonent.name, sizeof(newAbonent.name), stdin);
        removeN(newAbonent.name);

        printf("Введите фамилию нового абонента: ");
        fgets(newAbonent.second_name, sizeof(newAbonent.second_name), stdin);
        removeN(newAbonent.second_name);

        printf("Введите номер нового абонента: ");
        fgets(newAbonent.tel, sizeof(newAbonent.tel), stdin);
        removeN(newAbonent.tel);

        if (newAbonent.name[0] != '\0' && newAbonent.second_name[0] != '\0' &&
            newAbonent.tel[0] != '\0') {
          abonentList[newIndex] = newAbonent;
          printf("Успешно добавлено!\n\n");
        } else {
          printf("Произошла ошибка заполнения полей\n");
        }
        break;

      case '2':
        printf("Введите индекс абонента для удаления: ");
        int deleteIndex;
        char in[3];
        fgets(in, sizeof(in), stdin);
        sscanf(in, "%d", &deleteIndex);
        if ((deleteIndex > 0) && (deleteIndex <= LIST_SIZE) &&
            (abonentList[deleteIndex - 1].name[0] != '\0')) {
          struct abonent nullAbonent = {{0}, {0}, {0}};
          abonentList[deleteIndex - 1] = nullAbonent;
          break;
        } else {
          printf("Невозможно удалить абонента: абонент не существует\n");
          break;
        }

      case '3':
        printf("Введите имя абонента для поиска: ");
        char name[10];
        fgets(name, sizeof(name), stdin);
        removeN(name);
        char index = searchInList(1, name);
        if (index >= 0) {
          printf(
              "%d) %s %s: %s\n",
              index + 1,
              abonentList[index].name,
              abonentList[index].second_name,
              abonentList[index].tel);
        }

        break;

      case '4':
        printf("Вывод всех записей.\n");
        for (int i = 0; i < LIST_SIZE; ++i) {
          if (abonentList[i].name[0] != '\0') {
            printf(
                "%d) %s %s: %s\n",
                i + 1,
                abonentList[i].name,
                abonentList[i].second_name,
                abonentList[i].tel);
          }
        }
        break;
      case '5':
        printf("Выход\n");
        break;
      default:
        printf("Неизвестная команда\n");
        break;
    }
  }
  return 0;
}