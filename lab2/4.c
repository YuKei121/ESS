#include <stdio.h>
#include <stdlib.h>

int main() {
  int size = 0;
  int vars = 0;

  int value = 1;
  int sRow = 0, eRow = 0;
  int sCol = 0, eCol = 0;
  
  printf("Write arr size:\n");
  scanf("%d", &size);
  
  vars = size * size;
  
  eRow = eCol = size - 1;
  
  int** arr = malloc(sizeof(int*) * size);
  for (int i = 0; i < size; ++i) {
    arr[i] = malloc(sizeof(int) * size);
  }
  
  while ((sRow <= eRow) && (sCol <= eCol)) {
    for (int i = sCol; i <= eCol; ++i) {
      arr[sRow][i] = value++;
    }
    ++sRow;

    for (int i = sRow; i <= eRow; ++i) {
      arr[i][eCol] = value++;
    }
    --eCol;
    
    for (int i = eCol; i >= sCol; --i) {
      arr[eRow][i] = value++;
    }
    --eRow;

    for (int i = eRow; i >= sRow; --i) {
      arr[i][sCol] = value++;
    }
    ++sCol;
  }
  
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
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
