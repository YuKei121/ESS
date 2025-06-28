#include <stdio.h>
#include <stdlib.h>

int main() {
  int size = 0;
  
  printf("Write matrix size:\n");
  scanf("%d", &size);
  
  int** arr = malloc(sizeof(int*) * size);
  for (int i = 0; i < size; ++i) {
    arr[i] = malloc(sizeof(int) * size);
  }
  
  for (int j = 0; j < size; ++j) {
    for (int i = 0; i < size; ++i) {
      arr[i][j] = (((size - 1) - (i + j)) <= 0) ? 1 : 0;
      printf("%d ", arr[i][j]);
    }
    printf("\n");
  }
  
  for (int i = 0; i < size; ++i) {
    free(arr[i]);
  }
  free(arr);
  
  return 0;
}
