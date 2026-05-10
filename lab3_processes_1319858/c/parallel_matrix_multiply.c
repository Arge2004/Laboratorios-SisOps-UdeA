#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void multiply_matrix_sequential(int **a, int **b, int **c, int rows_a, int cols_a, int cols_b);
static int multiply_matrix_parallel(int **a, int **b, int **c, int rows_a, int cols_a, int cols_b, int k);

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

static double elapsed_seconds(const struct timespec *start, const struct timespec *end) {
    long sec = end->tv_sec - start->tv_sec;
    long nsec = end->tv_nsec - start->tv_nsec;
    return (double)sec + (double)nsec / 1e9;
}

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <matrizA.txt> <matrizB.txt> <salida.txt> <k>\n", argv[0]);
        return 1;
    }

    int **a = NULL;
    int **b = NULL;
    int **result = NULL;
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
    if (k > 1 && rows_a % k != 0) {
        fprintf(stderr, "Error: el numero de filas de A debe ser divisible por k\n");
        free_matrix(a, rows_a);
        free_matrix(b, rows_b);
        return 1;
    }

    result = alloc_matrix(rows_a, cols_b);
    if (result == NULL) {
        perror("alloc_matrix");
        free_matrix(a, rows_a);
        free_matrix(b, rows_b);
        return 1;
    }

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    if (k == 1) {
        multiply_matrix_sequential(a, b, result, rows_a, cols_a, cols_b);
    } else {
        if (multiply_matrix_parallel(a, b, result, rows_a, cols_a, cols_b, k) != 0) {
            free_matrix(a, rows_a);
            free_matrix(b, rows_b);
            free_matrix(result, rows_a);
            return 1;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    if (write_matrix_to_file(argv[3], result, rows_a, cols_b) != 0) {
        free_matrix(a, rows_a);
        free_matrix(b, rows_b);
        free_matrix(result, rows_a);
        return 1;
    }

    const char *mode = (k == 1) ? "sequential" : "parallel";
    printf("Mode: %s\n", mode);
    printf("Time (%d processes): %.3f seconds\n", k, elapsed_seconds(&start, &end));
    printf("Result saved in: %s\n", argv[3]);

    free_matrix(a, rows_a);
    free_matrix(b, rows_b);
    free_matrix(result, rows_a);
    return 0;
}

static void multiply_matrix_sequential(int **a, int **b, int **c, int rows_a, int cols_a, int cols_b) {
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

static int multiply_matrix_parallel(int **a, int **b, int **c, int rows_a, int cols_a, int cols_b, int k) {
    if (rows_a == 0 || cols_a == 0 || cols_b == 0 || k <= 0) {
        return 0;
    }

    int rows = rows_a;
    int cols = cols_b;
    int inner = cols_a;

    if (k > rows) {
        k = rows;
    }
    if (rows % k != 0) {
        fprintf(stderr, "Error: rows must be divisible by k\n");
        return -1;
    }

    size_t total = (size_t)rows * (size_t)cols;
    size_t bytes_to_share = total * sizeof(int);

    int shm_id = shmget(IPC_PRIVATE, bytes_to_share, IPC_CREAT | 0600);
    if (shm_id < 0) {
        perror("shmget");
        return -1;
    }

    int *shared = (int *)shmat(shm_id, NULL, 0);
    if (shared == (void *)-1) {
        perror("shmat");
        shmctl(shm_id, IPC_RMID, NULL);
        return -1;
    }

    memset(shared, 0, bytes_to_share);

    int base = rows / k;
    pid_t *pids = (pid_t *)malloc(sizeof(pid_t) * (size_t)k);
    if (pids == NULL) {
        perror("malloc");
        shmdt(shared);
        shmctl(shm_id, IPC_RMID, NULL);
        return -1;
    }

    int start = 0;
    for (int p = 0; p < k; p++) {
        int end = start + base;

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(pids);
            shmdt(shared);
            shmctl(shm_id, IPC_RMID, NULL);
            return -1;
        }

        if (pid == 0) {
            for (int i = start; i < end; i++) {
                for (int j = 0; j < cols; j++) {
                    int sum = 0;
                    for (int col = 0; col < inner; col++) {
                        sum += a[i][col] * b[col][j];
                    }
                    shared[i * cols + j] = sum;
                }
            }
            _exit(0);
        }

        pids[p] = pid;
        start = end;
    }

    int failed = 0;
    for (int i = 0; i < k; i++) {
        int status = 0;
        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            failed = 1;
            continue;
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            failed = 1;
        }
    }

    if (!failed) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                c[i][j] = shared[i * cols + j];
            }
        }
    }

    free(pids);
    shmdt(shared);
    shmctl(shm_id, IPC_RMID, NULL);

    return failed ? -1 : 0;
}
