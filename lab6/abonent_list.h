#ifndef ABONENT_LIST
#define ABONENT_LIST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 10

void removeN(char* str);

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

void freeEntireList(struct abonentList* node);
struct abonentList* addToList(struct abonentList* node);
struct abonentList* findInList(
    struct abonentList* const node,
    char name[ARRAY_SIZE]);
struct abonentList* deleteInList(
    struct abonentList* node,
    char name[ARRAY_SIZE]);
void printFromList(struct abonentList* const node);

#endif