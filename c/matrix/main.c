#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "matrix.h"

int main() {
  matrix_t A;
  vector_t x, b;

  matrix_init(&A, 3);  // Initialize a 3x3 matrix
  vector_init(&x, 3);  // Initialize vectors of size 3
  vector_init(&b, 3);

  // Example matrix and vector values
  A.vs[0][0] = 2.0;
  A.vs[0][1] = 1.0;
  A.vs[0][2] = -1.0;
  b.vs[0] = 8.0;
  A.vs[1][0] = -3.0;
  A.vs[1][1] = -1.0;
  A.vs[1][2] = 2.0;
  b.vs[1] = -11.0;
  A.vs[2][0] = -2.0;
  A.vs[2][1] = 1.0;
  A.vs[2][2] = 2.0;
  b.vs[2] = -3.0;

  matrix_t A_copy;
  matrix_copy(&A_copy, &A);  // Make a copy of A

  printf("Original Matrix A:\n");
  for (int i = 0; i < A.dim; i++) {
    for (int j = 0; j < A.dim; j++) {
      printf("%6.2f\t", A.vs[i][j]);
    }
    printf(" | %6.2f\n", b.vs[i]);
  }
  printf("\n");

  if (matrix_solve(&A, &x, &b)) {
    printf("Solution Vector x:\n");
    for (int i = 0; i < x.dim; i++) {
      printf("x[%d] = %6.2f\n", i, x.vs[i]);
    }
  } else {
    printf("Matrix is singular or no solution exists.\n");
  }

  printf("\nvalidate:\n");
  for (int i = 0; i < A_copy.dim; i++) {
    float dot = 0.0f;
    for (int j = 0; j < A_copy.dim; j++) {
      dot += A_copy.vs[i][j] * x.vs[j];
    }
    printf("b[%d] = %8.3f\n", i, dot);
  }
  return 0;
}
