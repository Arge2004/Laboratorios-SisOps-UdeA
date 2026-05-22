#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 5
#define PRODUCERS 2
#define CONSUMERS 2
#define ITEMS_PER_PRODUCER 10

typedef struct {
    int id;
    int count;
} ThreadArgs;

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;
sem_t empty_slots;
sem_t full_slots;
pthread_mutex_t mutex;

void *producer(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    for (int i = 0; i < args->count; i++) {
        int item = args->id * 100 + i;

        sem_wait(&empty_slots);
        pthread_mutex_lock(&mutex);

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&full_slots);

        printf("Productor %d pone %d\n", args->id, item);
    }
    return NULL;
}

void *consumer(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    for (int i = 0; i < args->count; i++) {
        sem_wait(&full_slots);
        pthread_mutex_lock(&mutex);

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty_slots);

        printf("Consumidor %d saca %d\n", args->id, item);
    }
    return NULL;
}

int main(void) {
    int total_items = PRODUCERS * ITEMS_PER_PRODUCER;
    int items_per_consumer = total_items / CONSUMERS;

    pthread_t prod_threads[PRODUCERS];
    pthread_t cons_threads[CONSUMERS];
    ThreadArgs prod_args[PRODUCERS];
    ThreadArgs cons_args[CONSUMERS];

    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < PRODUCERS; i++) {
        prod_args[i].id = i + 1;
        prod_args[i].count = ITEMS_PER_PRODUCER;
        pthread_create(&prod_threads[i], NULL, producer, &prod_args[i]);
    }

    for (int i = 0; i < CONSUMERS; i++) {
        cons_args[i].id = i + 1;
        cons_args[i].count = items_per_consumer;
        pthread_create(&cons_threads[i], NULL, consumer, &cons_args[i]);
    }

    for (int i = 0; i < PRODUCERS; i++) {
        pthread_join(prod_threads[i], NULL);
    }

    for (int i = 0; i < CONSUMERS; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);

    return 0;
}
