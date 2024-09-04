#ifndef SUMDIAG_H
#define SUMDIAG_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>            // anticipating threading


typedef struct {
  long rows;
  long cols;
  int *data;
} matrix_t;

typedef struct {
  long len;
  int *data;
} vector_t;

#define MGET(mat,i,j) ((mat).data[((i)*((mat).cols)) + (j)])
#define VGET(vec,i)   ((vec).data[(i)])

#define MSET(mat,i,j,x) ((mat).data[((i)*((mat).cols)) + (j)] = (x))
#define VSET(vec,i,x)   ((vec).data[(i)] = (x))


// sumdiag_util.c
int vector_init(vector_t *vec, long len);
int matrix_init(matrix_t *mat, long rows, long cols);
void vector_free_data(vector_t *vec);
void matrix_free_data(matrix_t *mat);
int vector_read_from_file(char *fname, vector_t *vec_ref);
int matrix_read_from_file(char *fname, matrix_t *mat_ref);
void vector_write(FILE *file, vector_t vec);
void matrix_write(FILE *file, matrix_t mat);
void vector_fill_sequential(vector_t vec);
void matrix_fill_sequential(matrix_t mat);
int mget(matrix_t *mat, int i, int j);
void mset(matrix_t *mat, int i, int j, int x);
int vget(vector_t *vec, int i);
void vset(vector_t *vec, int i, int x);

void pb_srand(unsigned long seed);
unsigned int pb_rand();
void vector_fill_random(vector_t vec, int max);
void matrix_fill_random(matrix_t mat, int max);

// sumdiag_base.c
int sumdiag_BASE(matrix_t mat, vector_t vec);

// sumdiag_optm.c
int sumdiag_OPTM(matrix_t mat, vector_t vec, int thread_count);

#endif
