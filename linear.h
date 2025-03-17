#ifndef LINEAR_H
#define LINEAR_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int rows;
    int cols;
    double* data; // used as 1d array (wraps around at each row)
} Matrix;

Matrix* matrix_new(int rows, int cols);
void matrix_init(Matrix* mat, const double vals[]);
void free_matrix(Matrix* mat);
void free_matrices(Matrix* mats[], int count);

// checks if matrix/data is null and if is in bounds.
// 1 = true, 0 = false
static inline int matrix_is_valid(const Matrix* mat, int row, int col) {
    if (mat == NULL || mat->data == NULL) {
        fprintf(stderr, "Error: NULL matrix or data\n");
        return 0;
    }
    
    if (row < 0 || row >= mat->rows || col < 0 || col >= mat->cols) {
        fprintf(stderr, "Error: Index out of bounds. (%d,%d) in %dx%d matrix\n", 
                row, col, mat->rows, mat->cols);
        return 0;
    }

    return 1;
}

static inline double matrix_get(const Matrix* mat, int row, int col) {
    if (!matrix_is_valid(mat, row, col)) {
        return 0.0;
    }

    return mat->data[row * mat->cols + col];
}

static inline void matrix_set(Matrix* mat, int row, int col, double val) {
    if (!matrix_is_valid(mat, row, col)) {
        return;
    }

    mat->data[row * mat->cols + col] = val;
}

void matrix_normalize(Matrix* mat);
Matrix* matrix_mult(const Matrix* left, const Matrix* right);
Matrix* matrix_subtract(const Matrix* left, const Matrix* right);
Matrix* cross_mult(const Matrix* left, const Matrix* right);
double dot_mult(const Matrix* left, const Matrix* right);

void matrix_print(const Matrix* mat, const char* name);

#endif // ! LINEAR_H
