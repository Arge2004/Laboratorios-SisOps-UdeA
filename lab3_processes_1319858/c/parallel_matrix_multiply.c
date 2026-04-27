#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void multiply_matrix_sequential(int **a, int **b, int **c, int rows_a, int cols_a, int cols_b);
int multiply_matrix_parallel(int **a, int **b, int **c, int rows_a, int cols_a, int cols_b, int k);

static int **alloc_matrix(int rows, int cols) {
    int **m = (int **)malloc((size_t)rows * sizeof(int *));
    if (m == NULL) {
        return NULL;
    }
    for (int i = 0; i < rows; i++) {
        m[i] = (int *)calloc((size_t)cols, sizeof(int));
        if (m[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(m[j]);
            }
            free(m);
            return NULL;
        }
    }
    return m;
}

static void free_matrix(int **m, int rows) {
    if (m == NULL) {
        return;
    }
    for (int i = 0; i < rows; i++) {
        free(m[i]);
    }
    free(m);
}

static int read_matrix_from_file(const char *path, int ***matrix, int *rows, int *cols) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    int **tmp = NULL;
    int row_cap = 0;
    int row_count = 0;
    int col_count = -1;

    char *line = NULL;
    size_t line_cap = 0;

    while (getline(&line, &line_cap, file) != -1) {
        char *saveptr = NULL;
        int row_vals[4096];
        int row_len = 0;

        for (char *token = strtok_r(line, " \t\r\n", &saveptr);
             token != NULL;
             token = strtok_r(NULL, " \t\r\n", &saveptr)) {
            errno = 0;
            char *endptr = NULL;
            long v = strtol(token, &endptr, 10);
            if (errno != 0 || *endptr != '\0') {
                fprintf(stderr, "Error: valor no numerico en %s\n", path);
                free(line);
                fclose(file);
                free_matrix(tmp, row_count);
                return -1;
            }
            row_vals[row_len++] = (int)v;
        }

        if (row_len == 0) {
            continue;
        }

        if (col_count == -1) {
            col_count = row_len;
        } else if (row_len != col_count) {
            fprintf(stderr, "Error: filas con diferente cantidad de columnas en %s\n", path);
            free(line);
            fclose(file);
            free_matrix(tmp, row_count);
            return -1;
        }

        if (row_count == row_cap) {
            int new_cap = row_cap == 0 ? 8 : row_cap * 2;
            int **new_tmp = (int **)realloc(tmp, (size_t)new_cap * sizeof(int *));
            if (new_tmp == NULL) {
                perror("realloc");
                free(line);
                fclose(file);
                free_matrix(tmp, row_count);
                return -1;
            }
            tmp = new_tmp;
            row_cap = new_cap;
        }

        tmp[row_count] = (int *)malloc((size_t)col_count * sizeof(int));
        if (tmp[row_count] == NULL) {
            perror("malloc");
            free(line);
            fclose(file);
            free_matrix(tmp, row_count);
            return -1;
        }
        for (int j = 0; j < col_count; j++) {
            tmp[row_count][j] = row_vals[j];
        }

        row_count++;
    }

    free(line);
    fclose(file);

    if (row_count == 0 || col_count <= 0) {
        fprintf(stderr, "Error: archivo vacio o invalido: %s\n", path);
        free_matrix(tmp, row_count);
        return -1;
    }

    *matrix = tmp;
    *rows = row_count;
    *cols = col_count;
    return 0;
}

static int write_matrix_to_file(const char *path, int **matrix, int rows, int cols) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (j > 0) {
                fputc(' ', file);
            }
            fprintf(file, "%d", matrix[i][j]);
        }
        fputc('\n', file);
    }

    fclose(file);
    return 0;
}

static int equal_matrices(int **a, int **b, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (a[i][j] != b[i][j]) {
                return 0;
            }
        }
    }
    return 1;
}

static double elapsed_seconds(const struct timespec *start, const struct timespec *end) {
    long sec = end->tv_sec - start->tv_sec;
    long nsec = end->tv_nsec - start->tv_nsec;
    return (double)sec + (double)nsec / 1e9;
}

int main(int argc, char **argv) {
    if (argc != 5 && argc != 6) {
        fprintf(stderr, "Uso: %s <matrizA.txt> <matrizB.txt> <salida.txt> <k> [full|seq|par]\n", argv[0]);
        return 1;
    }

    const char *mode = "full";
    if (argc == 6) {
        mode = argv[5];
    }
    if (strcmp(mode, "full") != 0 && strcmp(mode, "seq") != 0 && strcmp(mode, "par") != 0) {
        fprintf(stderr, "Error: modo invalido. Use full, seq o par\n");
        return 1;
    }

    int do_seq = (strcmp(mode, "full") == 0 || strcmp(mode, "seq") == 0);
    int do_par = (strcmp(mode, "full") == 0 || strcmp(mode, "par") == 0);

    int **a = NULL;
    int **b = NULL;
    int **seq = NULL;
    int **par = NULL;
    int rows_a = 0;
    int cols_a = 0;
    int rows_b = 0;
    int cols_b = 0;

    char *endptr = NULL;
    long k_long = strtol(argv[4], &endptr, 10);
    if (*argv[4] == '\0' || *endptr != '\0' || k_long <= 0) {
        fprintf(stderr, "Error: k debe ser un entero positivo\n");
        return 1;
    }
    int k = (int)k_long;

    if (read_matrix_from_file(argv[1], &a, &rows_a, &cols_a) != 0) {
        return 1;
    }
    if (read_matrix_from_file(argv[2], &b, &rows_b, &cols_b) != 0) {
        free_matrix(a, rows_a);
        return 1;
    }

    if (cols_a != rows_b) {
        fprintf(stderr, "Error: dimensiones incompatibles (columnas de A != filas de B)\n");
        free_matrix(a, rows_a);
        free_matrix(b, rows_b);
        return 1;
    }
    if (do_par && rows_a % k != 0) {
        fprintf(stderr, "Error: el numero de filas de A debe ser divisible por k\n");
        free_matrix(a, rows_a);
        free_matrix(b, rows_b);
        return 1;
    }

    if (do_seq) {
        seq = alloc_matrix(rows_a, cols_b);
    }
    if (do_par) {
        par = alloc_matrix(rows_a, cols_b);
    }

    if ((do_seq && seq == NULL) || (do_par && par == NULL)) {
        perror("alloc_matrix");
        free_matrix(a, rows_a);
        free_matrix(b, rows_b);
        free_matrix(seq, rows_a);
        free_matrix(par, rows_a);
        return 1;
    }

    struct timespec start_seq;
    struct timespec end_seq;
    struct timespec start_par;
    struct timespec end_par;

    if (do_seq) {
        clock_gettime(CLOCK_MONOTONIC, &start_seq);
        multiply_matrix_sequential(a, b, seq, rows_a, cols_a, cols_b);
        clock_gettime(CLOCK_MONOTONIC, &end_seq);
    }

    if (do_par) {
        clock_gettime(CLOCK_MONOTONIC, &start_par);
        if (multiply_matrix_parallel(a, b, par, rows_a, cols_a, cols_b, k) != 0) {
            free_matrix(a, rows_a);
            free_matrix(b, rows_b);
            free_matrix(seq, rows_a);
            free_matrix(par, rows_a);
            return 1;
        }
        clock_gettime(CLOCK_MONOTONIC, &end_par);
    }

    if (strcmp(mode, "full") == 0 && !equal_matrices(seq, par, rows_a, cols_b)) {
        fprintf(stderr, "Error: resultado paralelo no coincide con el secuencial\n");
        free_matrix(a, rows_a);
        free_matrix(b, rows_b);
        free_matrix(seq, rows_a);
        free_matrix(par, rows_a);
        return 1;
    }

    int **result = do_par ? par : seq;
    if (write_matrix_to_file(argv[3], result, rows_a, cols_b) != 0) {
        free_matrix(a, rows_a);
        free_matrix(b, rows_b);
        free_matrix(seq, rows_a);
        free_matrix(par, rows_a);
        return 1;
    }

    double seq_seconds = do_seq ? elapsed_seconds(&start_seq, &end_seq) : 0.0;
    double par_seconds = do_par ? elapsed_seconds(&start_par, &end_par) : 0.0;
    double speedup = (do_seq && do_par && par_seconds > 0.0) ? seq_seconds / par_seconds : 0.0;

    printf("Sequential time: %.3f seconds\n", seq_seconds);
    printf("Parallel time (%d processes): %.3f seconds\n", k, par_seconds);
    printf("Speedup: %.2fx\n", speedup);
    printf("Result saved in: %s\n", argv[3]);

    free_matrix(a, rows_a);
    free_matrix(b, rows_b);
    free_matrix(seq, rows_a);
    free_matrix(par, rows_a);
    return 0;
}
