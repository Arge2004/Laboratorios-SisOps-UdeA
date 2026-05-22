#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define PRODUCERS 2
#define CONSUMERS 2
#define ITEMS_PER_PRODUCER 10

typedef struct Node {
    int value;
    struct Node *next;
} Node;

typedef struct {
    Node *front;
    Node *rear;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
} ThreadSafeQueue;

typedef struct {
    int id;
    ThreadSafeQueue *q;
    int count;
} ThreadArgs;

void queue_init(ThreadSafeQueue *q) {
    q->front = NULL;
    q->rear = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

void queue_destroy(ThreadSafeQueue *q) {
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_empty);
}

void enqueue(ThreadSafeQueue *q, int item) {
    Node *node = malloc(sizeof(Node));
    if (node == NULL) {
        perror("malloc");
        exit(1);
    }
    node->value = item;
    node->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->rear == NULL) {
        q->front = node;
        q->rear = node;
    } else {
        q->rear->next = node;
        q->rear = node;
    }
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

int dequeue(ThreadSafeQueue *q) {
    pthread_mutex_lock(&q->lock);
    while (q->front == NULL) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
    Node *node = q->front;
    int item = node->value;
    q->front = node->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    pthread_mutex_unlock(&q->lock);

    free(node);
    return item;
}

void *producer(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    for (int i = 0; i < args->count; i++) {
        int item = args->id * 100 + i;
        enqueue(args->q, item);
        printf("Productor %d encola %d\n", args->id, item);
    }
    return NULL;
}

void *consumer(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    for (int i = 0; i < args->count; i++) {
        int item = dequeue(args->q);
        printf("Consumidor %d saca %d\n", args->id, item);
    }
    return NULL;
}

int main(void) {
    ThreadSafeQueue q;
    queue_init(&q);

    int total_items = PRODUCERS * ITEMS_PER_PRODUCER;
    int items_per_consumer = total_items / CONSUMERS;

    pthread_t prod_threads[PRODUCERS];
    pthread_t cons_threads[CONSUMERS];
    ThreadArgs prod_args[PRODUCERS];
    ThreadArgs cons_args[CONSUMERS];

    for (int i = 0; i < PRODUCERS; i++) {
        prod_args[i].id = i + 1;
        prod_args[i].q = &q;
        prod_args[i].count = ITEMS_PER_PRODUCER;
        pthread_create(&prod_threads[i], NULL, producer, &prod_args[i]);
    }

    for (int i = 0; i < CONSUMERS; i++) {
        cons_args[i].id = i + 1;
        cons_args[i].q = &q;
        cons_args[i].count = items_per_consumer;
        pthread_create(&cons_threads[i], NULL, consumer, &cons_args[i]);
    }

    for (int i = 0; i < PRODUCERS; i++) {
        pthread_join(prod_threads[i], NULL);
    }

    for (int i = 0; i < CONSUMERS; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    queue_destroy(&q);
    return 0;
}
