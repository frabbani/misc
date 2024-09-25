#include <math.h>

#define SWAP(a, b) do { \
    __typeof__(a) t; \
    t = a; \
    a = b; \
    b = t; \
} while (0)

#define MAX_SIZE 128

typedef struct {
  int dim;
  double vs[MAX_SIZE][MAX_SIZE];
} matrix_t;

typedef struct {
  int dim;
  double vs[MAX_SIZE];
} vector_t;

int vector_init(vector_t *x, int dim) {
  x->dim = dim;
  for (int i = 0; i < MAX_SIZE; i++)
    x->vs[i] = 0.0;
  return 1;
}

void vector_copy(vector_t *to, const vector_t *from) {
  to->dim = from->dim;
  for (int i = 0; i < from->dim; i++)
    to->vs[i] = from->vs[i];
}

void matrix_init(matrix_t *A, int dim) {
  A->dim = dim;
  for (int i = 0; i < A->dim; i++)
    for (int j = 0; j < A->dim; j++)
      A->vs[i][j] = 0.0;
}

void matrix_copy(matrix_t *A, const matrix_t *B) {
  A->dim = B->dim;
  for (int i = 0; i < A->dim; i++)
    for (int j = 0; j < A->dim; j++)
      A->vs[i][j] = B->vs[i][j];
}

void matrix_pivot(matrix_t *A, int row_nos[], int r) {
  float p, q;
  p = fabs(A->vs[row_nos[r]][r]);

  for (int i = r + 1; i < A->dim; i++) {
    q = fabs(A->vs[row_nos[i]][r]);
    if (q > p) {
      SWAP(row_nos[r], row_nos[i]);
      p = q;
    }
  }
}

void matrix_reduce(matrix_t *A, vector_t *b, int row_nos[], int i, int j) {
  if (i == j || i < 0 || i >= A->dim || j < 0 || j >= A->dim)
    return;

  int ri = row_nos[i];
  int rj = row_nos[j];

  double s = A->vs[ri][j];
  double t = A->vs[rj][j];

  if (fabsf(s) < 1e-8f || fabsf(t) < 1e-8f)
    return;

  float st_inv = 1.0f / (s * t);

  for (int k = j; k < A->dim; k++)
    A->vs[ri][k] = (t * A->vs[ri][k] - s * A->vs[rj][k]) * st_inv;

  b->vs[ri] = (t * b->vs[ri] - s * b->vs[rj]) * st_inv;
}

void matrix_solve_u(const matrix_t *U, vector_t *x, const vector_t *b,
                    int row_nos[]) {
  if (U->dim != b->dim)
    return;

  int dim = U->dim;
  vector_init(x, dim);

  for (int i = dim - 1; i >= 0; i--) {
    int k = row_nos[i];
    x->vs[i] = b->vs[k];
    for (int j = i + 1; j < dim; j++)
      x->vs[i] -= U->vs[k][j] * x->vs[j];
    x->vs[i] /= U->vs[k][i];
  }
}

int matrix_solve(matrix_t *A, vector_t *x, vector_t *b) {
  int n, row_nos[MAX_SIZE];

  n = A->dim;
  for (int i = 0; i < n; i++)
    row_nos[i] = i;

  for (int i = 0; i < n - 1; i++) {
    matrix_pivot(A, row_nos, i);
    for (int j = i + 1; j < n; j++)
      matrix_reduce(A, b, row_nos, j, i);
  }

  float det = 1.0f;
  for (int i = 0; i < n; i++)
    det *= A->vs[row_nos[i]][i];

  if (fabs(det) < 1e-8f)
    return 0;

  matrix_solve_u(A, x, b, row_nos);
  return 1;
}

void matrix_multiply(const matrix_t *A, const vector_t *x, vector_t *b) {
  if (A->dim != x->dim || A->dim != b->dim) {
    return;
  }

  // Perform matrix-vector multiplication
  for (int i = 0; i < A->dim; ++i) {
    b->vs[i] = 0.0f;
    for (int j = 0; j < A->dim; ++j) {
      b->vs[i] += A->vs[i][j] * x->vs[j];
    }
  }
}

void vector_add(const vector_t *a, const vector_t *b, vector_t *x) {
  if (a->dim != b->dim || a->dim != x->dim) {
    return;
  }

  // Perform vector addition
  for (int i = 0; i < a->dim; ++i) {
    x->vs[i] = a->vs[i] + b->vs[i];
  }
}
