# Operating Systems Course

# University of Antioquia

## Practice # 4 - Synchronization Mechanisms in Operating

## Systems

## Objective

To understand and implement synchronization mechanisms—locks, condition
variables, and semaphores—to solve concurrency problems in multi-threaded
programs.

## Introduction

```
In multi-threaded programs, shared resources must be accessed in a controlled
manner to avoid race conditions. This lab explores three synchronization mecha-
nisms:
```
- **Locks (Mutexes)** : Ensure mutual exclusion.
- **Condition Variables** : Allow threads to wait for a condition to become
    true.
- **Semaphores** : General synchronization primitives that can control access
    to resources.

## Task 1: Implementing a Thread-Safe Queue using Locks

## and Condition Variables

```
Description
Implement a thread-safe queue where:
```
- Multiple threads can enqueue and dequeue items.
- If the queue is empty, threads trying to dequeue must wait until an item
    is available.

```
Requirements
```
- Use pthread_mutex_t for locking.
- Use pthread_cond_t for signaling when the queue is no longer empty.


**Sample Code Structure**

#include <pthread.h>

typedef struct {
int *items;
int front, rear, size;
pthread_mutex_t lock;
pthread_cond_t not_empty;
} ThreadSafeQueue;

void enqueue(ThreadSafeQueue *q, int item) {
pthread_mutex_lock(&q->lock);
// Add item to queue
pthread_cond_signal(&q->not_empty);
pthread_mutex_unlock(&q->lock);
}

int dequeue(ThreadSafeQueue *q) {
pthread_mutex_lock(&q->lock);
while (queue_empty(q)) {
pthread_cond_wait(&q->not_empty, &q->lock);
}
int item = // remove item from queue
pthread_mutex_unlock(&q->lock);
return item;
}

**Tasks**

- Complete the implementation.
- Test with multiple producer and consumer threads.

## Task 2: Solving the Producer-Consumer Problem using

## Semaphores

**Description**

Implement the bounded-buffer producer-consumer problem using semaphores:

- Producers add items to a buffer of fixed size.
- Consumers remove items.
- Ensure no buffer overflow/underflow.


**Requirements**

- Use sem_t for synchronization.
- Use two semaphores:
    **-** empty (counts empty slots).
    **-** full (counts filled slots).

**Sample Code Structure**

#include <semaphore.h>

#define BUFFER_SIZE 10

int buffer[BUFFER_SIZE];
sem_t empty, full;
pthread_mutex_t mutex;

void *producer(void *arg) {
while (1) {
int item = produce_item();
sem_wait(&empty); // Wait if buffer is full
pthread_mutex_lock(&mutex);
// Add item to buffer
pthread_mutex_unlock(&mutex);
sem_post(&full); // Signal that an item is available
}
}

void *consumer(void *arg) {
while (1) {
sem_wait(&full); // Wait if buffer is empty
pthread_mutex_lock(&mutex);
int item = // Remove item from buffer
pthread_mutex_unlock(&mutex);
sem_post(&empty); // Signal a slot is free
consume_item(item);
}
}

**Task**

- Complete the implementation.
- Test with multiple producers and consumers.


## Task 3: The Dining Philosophers Problem

**Description**

Implement a solution to the Dining Philosophers problem where:

- Philosophers alternate between thinking and eating.
- Deadlock and starvation must be prevented.

Possible Solutions:

- Use a mutex for each fork (but avoid deadlock).
- Use a semaphore to limit concurrent eaters.

**Task**

- Implement a deadlock-free solution.
- Test with 5 philosophers.

## Submission Guidelines

- Compress and submit your source code in C/C++ and Go (queue.x,
    producer_consumer.x, dining_philosophers.x).
- Include a **README** explaining your approach.
- Provide sample outputs showing correct synchronization.

## Evaluation Criteria

```
Criteria Points
Correct use of locks and condition variables 30
Proper semaphore implementation 30
Deadlock-free solution 20
Code readability and documentation 20
```

