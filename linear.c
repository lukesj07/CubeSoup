#include "linear.h"

// creates a row-major matrix struct.
// make sure to free after done with matrix.
// returns null matrix struct if error.
Matrix* matrix_new(int rows, int cols) {
    Matrix* mat = (Matrix*)malloc(sizeof(Matrix));
    if (mat == NULL) {
        fprintf(stderr, "Error allocating memory for matrix struct\n");
        return NULL;
    }
    mat->rows = rows;
    mat->cols = cols;
    mat->data = (double*) malloc(rows * cols * sizeof(double));
    if (mat->data == NULL) {
        fprintf(stderr, "Error allocating memory for matrix data\n");
        free(mat);
        return NULL;
    }
    return mat;
}

// ensure that vals[] matches the size of the matrix.
void matrix_init(Matrix* mat, const double vals[]) {
    if (!matrix_is_valid(mat, 0, 0) || vals == NULL) {
        return;
    }

    for (int i = 0; i < mat->rows * mat->cols; i++) {
        mat->data[i] = vals[i];
    }
}

// set pointer to null after using.
void free_matrix(Matrix* mat) {
    if (mat == NULL) {
        return;
    }

    free(mat->data);
    free(mat);
}

// eg. matrix_free((Matrix*[]) {a, b, c}, 3);
void free_matrices(Matrix* mats[], int count) {
    for (int i = 0; i < count; i++) {
        free_matrix(mats[i]);
        mats[i] = NULL;
    }
}

void matrix_normalize(Matrix* mat) {
    if (!matrix_is_valid(mat, 0, 0)) {
        return;
    }

    for (int col = 0; col < mat->cols; col++) {
        double sum = 0;
        for (int row = 0; row < mat->rows; row++) {
            sum += pow(matrix_get(mat, row, col), 2);
        }
        sum = sqrt(sum);
        for (int row = 0; row < mat->rows; row++) {
            matrix_set(mat, row, col, matrix_get(mat, row, col) / sum);
        }
    }
}

// make sure to free result after done.
// returns null matrix struct if error.
Matrix* matrix_mult(const Matrix* left, const Matrix* right) {
    if (!matrix_is_valid(left, 0, 0) || !matrix_is_valid(right, 0, 0)) {
        return NULL;
    }

    if (right->rows != left->cols) {
        fprintf(stderr, "Invalid sizes during matrix multiplication: %dx%d * %dx%d\n",
                left->rows, left->cols, right->rows, right->cols);
        return NULL;
    }

    Matrix* result = matrix_new(left->rows, right->cols);

    for (int row = 0; row < result->rows; row++) {
        for (int col = 0; col < result->cols; col++) {
            double element = 0;
            for (int pair = 0; pair < right->rows; pair++) { // pair of corresponding elements in matrices
                element += matrix_get(left, row, pair) * matrix_get(right, pair, col);
            }
            matrix_set(result, row, col, element);
        }
    }
    return result;
}

// make sure to free result after done.
// returns null matrix struct if error.
Matrix* matrix_subtract(const Matrix* left, const Matrix* right) {
    if (!matrix_is_valid(left, 0, 0) || !matrix_is_valid(right, 0, 0)) {
        return NULL;
    }

    if (left->cols != right->cols || left->rows != right->rows) {
        fprintf(stderr, "Matrix sizes must match for subtraction: %dx%d - %dx%d\n",
                left->rows, left->cols, right->rows, right->cols);
        return NULL;
    }

    Matrix* result = matrix_new(left->rows, left->cols);

    for (int row = 0; row < left->rows; row++) {
        for (int col = 0; col < left->cols; col++) {
            matrix_set(result, row, col, matrix_get(left, row, col) - matrix_get(right, row, col));
        }
    }
    return result;
}

// make sure to free result after done.
// returns null matrix struct if error.
Matrix* cross_mult(const Matrix* left, const Matrix* right) {
    if (!matrix_is_valid(left, 0, 0) || !matrix_is_valid(right, 0, 0)) {
        return NULL;
    }

    if (left->cols != 1 || left->rows != 3 || right->cols != 1 || right->rows != 3) {
        fprintf(stderr, "Invalid sizes for cross product: %dx%d x %dx%d\n",
                left->rows, left->cols, right->rows, right->cols);
        return NULL;
    }
    
    Matrix* result = matrix_new(3, 1);
    const double product[] = {
        matrix_get(left, 1, 0) * matrix_get(right, 2, 0) - matrix_get(left, 2, 0) * matrix_get(right, 1, 0),
        matrix_get(left, 2, 0) * matrix_get(right, 0, 0) - matrix_get(left, 0, 0) * matrix_get(right, 2, 0),
        matrix_get(left, 0, 0) * matrix_get(right, 1, 0) - matrix_get(left, 1, 0) * matrix_get(right, 0, 0)
    };
    matrix_init(result, product);
    return result;
}

double dot_mult(const Matrix* left, const Matrix* right) {
    if (!matrix_is_valid(left, 0, 0) || !matrix_is_valid(right, 0, 0)) {
        return 0.0;
    }
    if (left->cols != 1 || right->cols != 1 || left->rows != right->rows) {
        fprintf(stderr, "Invalid sizes for dot product: %dx%d . %dx%d\n",
                left->rows, left->cols, right->rows, right->cols);
        return 0.0;
    }
    double sum = 0;
    for (int row = 0; row < left->rows; row++) {
        sum += matrix_get(left, row, 0) * matrix_get(right, row, 0);
    }
    return sum;
}


void matrix_print(const Matrix* mat, const char* name) {
    if (mat == NULL || mat->data == NULL || mat->rows == 0 || mat->cols == 0) {
        printf("%s: [NULL/INVALID MATRIX]\n", name);
        return;
    }
    
    printf("%s: %dx%d matrix\n", name, mat->rows, mat->cols);

    for (int i = 0; i < mat->rows; i++) {
        printf("  [");
        for (int j = 0; j < mat->cols; j++) {
            double value = mat->data[i * mat->cols + j];
            if (j == mat->cols - 1) {
                printf("%7.3f", value);
            } else {
                printf("%7.3f, ", value);
            }
        }
        printf("]\n");
    }
}
