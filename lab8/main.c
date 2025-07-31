#include <stdio.h>
#include "calc.h"

int getIntFromUser() {
  int result = 0;
  char buf[100];

  printf("Введите целое число: ");
  fgets(buf, sizeof(buf), stdin);
  if (sscanf(buf, "%d", &result) != 1) {
    printf("Введено некорректное число! Оно будет заменено на 0\n");
  }
  return result;
}

void addFunc() {
  int num1 = getIntFromUser();
  int num2 = getIntFromUser();
  printf("%d + %d = %d\n", num1, num2, add(num1, num2));
}

void subFunc() {
  int num1 = getIntFromUser();
  int num2 = getIntFromUser();
  printf("%d - %d = %d\n", num1, num2, sub(num1, num2));
}

void mulFunc() {
  int num1 = getIntFromUser();
  int num2 = getIntFromUser();
  printf("%d * %d = %d\n", num1, num2, mul(num1, num2));
}

void divFunc() {
  int num1 = getIntFromUser();
  int num2 = getIntFromUser();
  printf("%d / %d = %d\n", num1, num2, div(num1, num2));
}

int main() {
  char userInput[3];
  char command = 0;
  printf(
      "\n"
      "1) Сложнение\n"
      "2) Вычитание\n"
      "3) Умножение\n"
      "4) Деление\n"
      "5) Выход\n");
  while (command != '5') {
    // scanf("%c", &userInput);
    printf("Введите команду: ");
    fgets(userInput, sizeof(userInput), stdin);
    sscanf(userInput, "%c", &command);
    switch (command) {
      case '1':
        addFunc();
        break;
      case '2':
        subFunc();
        break;
      case '3':
        mulFunc();
        break;
      case '4':
        divFunc();
        break;
      case '5':
        break;
      default:
        printf("Неизвестная команда!\n");
    }
  }
  return 0;
}