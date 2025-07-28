#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 10

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
  char name[ARRAY_SIZE];
  char second_name[ARRAY_SIZE];
  char tel[ARRAY_SIZE];
};

struct abonentList {
  struct abonent abonent;
  struct abonentList* next;
  struct abonentList* prev;
};

void freeEntireList(struct abonentList* node) {
  if (node != NULL) {
    freeEntireList(node->next);
  }
  free(node);
}

struct abonentList* addToList(struct abonentList* node) {
  struct abonentList* newNode = malloc(sizeof(struct abonentList));
  if (newNode == NULL) {
    perror("malloc error");
    freeEntireList(node);
    exit(EXIT_FAILURE);
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
    newNode->abonent = newAbonent;
    if (node != NULL) {
      node->prev = newNode;
    }
    newNode->next = node;
    printf("Added successfully!\n");
    return newNode;
  } else {
    perror("Произошла ошибка заполнения полей абонента");
    free(newNode);
    freeEntireList(node);
    exit(EXIT_FAILURE);
  }
}

struct abonentList* findInList(
    struct abonentList* const node,
    char name[ARRAY_SIZE]) {
  struct abonentList* foundNode = NULL;
  if (node != NULL) {
    if (strcmp(node->abonent.name, name) == 0) {
      return node;
    } else {
      foundNode = findInList(node->next, name);
    }
  }
  return foundNode;
}

struct abonentList* deleteInList(
    struct abonentList* node,
    char name[ARRAY_SIZE]) {
  struct abonentList* foundNode = NULL;
  if (node != NULL) {
    if (strcmp(node->abonent.name, name) == 0) {
      if (node->prev != NULL) {
        node->prev->next = node->next;
      }
      if (node->next != NULL) {
        node->next->prev = node->prev;
      }
      foundNode = node->prev;
      free(node);
    } else {
      foundNode = deleteInList(node->next, name);
    }
  }
  return foundNode;
}

void printFromList(struct abonentList* const node) {
  if (node != NULL) {
    struct abonent ab = node->abonent;
    printf("%s %s: %s\n", ab.name, ab.second_name, ab.tel);
    printFromList(node->next);
  }
}
