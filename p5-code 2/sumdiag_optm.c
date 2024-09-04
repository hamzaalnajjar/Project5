#include "sumdiag.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

// ==== Matrix Diagonal Sum Benchmark Version 5 ====
//------ Tuned for ODD grace.umd.edu machines -----
//Running with 5 sizes (max 8192) and 4 thread_counts (max 4)
//  SIZE   BASE #T   OPTM  SPDUP POINTS 
// 1024  0.029  1  0.017   1.68   0.17 
//             2  0.012   2.42   0.29 
//             3  0.014   2.08   0.24 
//             4  0.020   1.43   0.12 
// 1101  0.030  1  0.019   1.56   0.16 
//             2  0.013   2.32   0.30 
//             3  0.018   1.69   0.19 
//             4  0.019   1.53   0.15 
// 2048  0.108  1  0.068   1.58   0.30 
//             2  0.035   3.10   0.74 
//             3  0.042   2.57   0.62 
//             4  0.041   2.63   0.64 
// 4099  0.476  1  0.386   1.23   0.28 
//             2  0.194   2.45   1.18 
//             3  0.235   2.03   0.93 
//             4  0.213   2.23   1.06 
//8192  2.911  1  2.342   1.24   0.57 
//             2  1.208   2.41   2.31 
//             3  1.265   2.30   2.19 
//             4  0.941   3.09   2.97 
// RAW POINTS: 15.38
//	       TOTAL POINTS: 15 / 30 //
	      

// Structure to pass necessary data to each thread
typedef struct {
    int start_diag;
    int end_diag;
    matrix_t mat;
    vector_t vec;
    pthread_mutex_t *mutex;
} thread_data_t;

// Function to calculate sums of specified diagonals of a matrix
void *calculate_diagonal_sums(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    matrix_t mat = data->mat;
    vector_t vec = data->vec;

    for (int d = data->start_diag; d < data->end_diag; d++) {
        int sum = 0;
        int r, c;
        if (d < mat.rows) {
            r = mat.rows - 1 - d;
            c = 0;
        } else {
            r = 0;
            c = d - mat.rows + 1;
        }
        while (r < mat.rows && c < mat.cols) {
            sum += MGET(mat, r, c);
            r++;
            c++;
        }
        pthread_mutex_lock(data->mutex);
        VSET(vec, d, sum);
        pthread_mutex_unlock(data->mutex);
    }
    return NULL;
}

// Optimized function to calculate diagonal sums using multiple threads
int sumdiag_OPTM(matrix_t mat, vector_t vec, int thread_count) {
    if (vec.len != (mat.rows + mat.cols - 1)) {
        printf("sumdiag_optm: size mismatch\n");
        return 1;
    }

    pthread_t *threads = malloc(thread_count * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(thread_count * sizeof(thread_data_t));
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    int diags = mat.rows + mat.cols - 1;
    int diags_per_thread = diags / thread_count;
    int extra_diags = diags % thread_count;

    for (int i = 0; i < thread_count; i++) {
        int start_diag = i * diags_per_thread + (i < extra_diags ? i : extra_diags);
        int end_diag = start_diag + diags_per_thread + (i < extra_diags ? 1 : 0);
        thread_data[i] = (thread_data_t){start_diag, end_diag, mat, vec, &mutex};
        pthread_create(&threads[i], NULL, calculate_diagonal_sums, &thread_data[i]);
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    free(threads);
    free(thread_data);
    return 0;
}
