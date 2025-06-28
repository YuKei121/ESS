#include <stdio.h>
#include <stdlib.h>

void removeN(char *str) {
  while (*str != '\0') {
    if (*str == '\n') {
      *str = '\0';
      break;
    }
    str++;
  }
}

char* search(char* str, char* substr) {
  if (*substr == '\0') {
    return NULL;
  }
  
  
  while (*str != '\0') {
    char* ptrStr = str;
    char* ptrSubstr = substr;
    
    while (*ptrSubstr != '\0' && *ptrSubstr == *ptrStr) {
      ptrStr++;
      ptrSubstr++; 
    }
    
    if (*ptrSubstr == '\0') {
      return str;
    }
    
    str++;
  }
    

  return NULL;
}

int main() {
  char str[100];
  char substr[100];
  
  printf("Print the string: ");
  fgets(str, sizeof(str), stdin);
  
  removeN(str);
  
  printf("Print the substring: ");
  fgets(substr, sizeof(substr), stdin);
  
  removeN(substr);
  
  printf("Trying to find %s in %s...\n", substr, str);
  
  char* ptr = search(str, substr);
  
  if (ptr != NULL) {
    printf("Found\n");
  } else {
    printf("Not Found\n");
  }
  
  return 0;
}
