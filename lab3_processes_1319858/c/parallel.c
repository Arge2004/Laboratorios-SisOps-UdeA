#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int multiply_matrix_parallel(int **a, int **b, int **c, int rows_a, int cols_a, int cols_b, int k) {
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
