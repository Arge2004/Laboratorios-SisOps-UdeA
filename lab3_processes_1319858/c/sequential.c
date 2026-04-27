#include <stddef.h>

void multiply_matrix_sequential(int **a, int **b, int **c, int rows_a, int cols_a, int cols_b) {
    for (int i = 0; i < rows_a; i++) {
        for (int j = 0; j < cols_b; j++) {
            int sum = 0;
            for (int k = 0; k < cols_a; k++) {
                sum += a[i][k] * b[k][j];
            }
            c[i][j] = sum;
        }
    }
}
