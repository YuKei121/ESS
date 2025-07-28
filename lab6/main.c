#include <stdio.h>
#include <string.h>

#include "abonent_list.h"

int main() {
  struct abonentList* list;
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
        list = addToList(list);
        break;
      case '2':
        char deleteName[ARRAY_SIZE] = {0};
        printf("Введите имя удаляемого абонента: ");
        fgets(deleteName, sizeof(deleteName), stdin);
        removeN(deleteName);
        list = deleteInList(list, deleteName);
        break;
      case '3':
        char searchName[ARRAY_SIZE] = {0};
        printf("Введите имя искомого абонента: ");
        fgets(searchName, sizeof(searchName), stdin);
        removeN(searchName);
        struct abonentList* foundNode = findInList(list, searchName);
        printf(
            "%s %s: %s\n",
            foundNode->abonent.name,
            foundNode->abonent.second_name,
            foundNode->abonent.tel);
        break;
      case '4':
        printFromList(list);
        break;
      case '5':
        printf("Выход\n");
        break;
      default:
        printf("Неизвестная команда\n");
        break;
    }
  }
  if (list != NULL) {
    freeEntireList(list);
  }
  return 0;
}
