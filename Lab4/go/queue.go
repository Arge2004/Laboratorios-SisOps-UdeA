package main

import (
    "fmt"
    "sync"
)

type ThreadSafeQueue struct {
    items    []int
    mu       sync.Mutex
    notEmpty *sync.Cond
}

func NewQueue() *ThreadSafeQueue {
    q := &ThreadSafeQueue{}
    q.notEmpty = sync.NewCond(&q.mu)
    return q
}

func (q *ThreadSafeQueue) Enqueue(item int) {
    q.mu.Lock()
    q.items = append(q.items, item)
    q.notEmpty.Signal()
    q.mu.Unlock()
}

func (q *ThreadSafeQueue) Dequeue() int {
    q.mu.Lock()
    for len(q.items) == 0 {
        q.notEmpty.Wait()
    }
    item := q.items[0]
    q.items = q.items[1:]
    q.mu.Unlock()
    return item
}

func producer(id int, q *ThreadSafeQueue, count int, wg *sync.WaitGroup) {
    defer wg.Done()
    for i := 0; i < count; i++ {
        item := id*100 + i
        q.Enqueue(item)
        fmt.Printf("Productor %d encola %d\n", id, item)
    }
}

func consumer(id int, q *ThreadSafeQueue, count int, wg *sync.WaitGroup) {
    defer wg.Done()
    for i := 0; i < count; i++ {
        item := q.Dequeue()
        fmt.Printf("Consumidor %d saca %d\n", id, item)
    }
}

func main() {
    q := NewQueue()

    var wg sync.WaitGroup
    producers := 2
    consumers := 2
    itemsPerProducer := 10
    itemsPerConsumer := (producers * itemsPerProducer) / consumers

    for i := 1; i <= producers; i++ {
        wg.Add(1)
        go producer(i, q, itemsPerProducer, &wg)
    }

    for i := 1; i <= consumers; i++ {
        wg.Add(1)
        go consumer(i, q, itemsPerConsumer, &wg)
    }

    wg.Wait()
}
