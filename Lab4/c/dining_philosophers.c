#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PHILOSOPHERS 5
#define MEALS 3

typedef struct {
    int id;
    int left;
    int right;
} Philosopher;

pthread_mutex_t forks[PHILOSOPHERS];
sem_t room;

void *philosopher(void *arg) {
    Philosopher *p = (Philosopher *)arg;
    for (int i = 0; i < MEALS; i++) {
        printf("Filosofo %d piensa\n", p->id);
        usleep(50000);

        sem_wait(&room);

        pthread_mutex_lock(&forks[p->left]);
        printf("Filosofo %d toma tenedor %d\n", p->id, p->left + 1);

        pthread_mutex_lock(&forks[p->right]);
        printf("Filosofo %d toma tenedor %d\n", p->id, p->right + 1);

        printf("Filosofo %d come (%d)\n", p->id, i + 1);
        usleep(50000);

        pthread_mutex_unlock(&forks[p->right]);
        printf("Filosofo %d deja tenedor %d\n", p->id, p->right + 1);

        pthread_mutex_unlock(&forks[p->left]);
        printf("Filosofo %d deja tenedor %d\n", p->id, p->left + 1);

        sem_post(&room);
    }
    return NULL;
}

int main(void) {
    pthread_t threads[PHILOSOPHERS];
    Philosopher philosophers[PHILOSOPHERS];

    for (int i = 0; i < PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }

    sem_init(&room, 0, PHILOSOPHERS - 1);

    for (int i = 0; i < PHILOSOPHERS; i++) {
        philosophers[i].id = i + 1;
        philosophers[i].left = i;
        philosophers[i].right = (i + 1) % PHILOSOPHERS;
        pthread_create(&threads[i], NULL, philosopher, &philosophers[i]);
    }

    for (int i = 0; i < PHILOSOPHERS; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    sem_destroy(&room);
    return 0;
}
