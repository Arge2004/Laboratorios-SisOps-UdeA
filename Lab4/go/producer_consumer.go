package main

import (
    "fmt"
    "sync"
)

func producer(id int, buffer []int, empty chan struct{}, full chan struct{}, mu *sync.Mutex, in *int, count int, wg *sync.WaitGroup) {
    defer wg.Done()
    for i := 0; i < count; i++ {
        item := id*100 + i
        <-empty
        mu.Lock()
        buffer[*in] = item
        *in = (*in + 1) % len(buffer)
        mu.Unlock()
        full <- struct{}{}
        fmt.Printf("Productor %d pone %d\n", id, item)
    }
}

func consumer(id int, buffer []int, empty chan struct{}, full chan struct{}, mu *sync.Mutex, out *int, count int, wg *sync.WaitGroup) {
    defer wg.Done()
    for i := 0; i < count; i++ {
        <-full
        mu.Lock()
        item := buffer[*out]
        *out = (*out + 1) % len(buffer)
        mu.Unlock()
        empty <- struct{}{}
        fmt.Printf("Consumidor %d saca %d\n", id, item)
    }
}

func main() {
    bufferSize := 5
    producers := 2
    consumers := 2
    itemsPerProducer := 10
    itemsPerConsumer := (producers * itemsPerProducer) / consumers

    buffer := make([]int, bufferSize)
    in := 0
    out := 0
    var mu sync.Mutex

    empty := make(chan struct{}, bufferSize)
    full := make(chan struct{}, bufferSize)
    for i := 0; i < bufferSize; i++ {
        empty <- struct{}{}
    }

    var wg sync.WaitGroup
    for i := 1; i <= producers; i++ {
        wg.Add(1)
        go producer(i, buffer, empty, full, &mu, &in, itemsPerProducer, &wg)
    }

    for i := 1; i <= consumers; i++ {
        wg.Add(1)
        go consumer(i, buffer, empty, full, &mu, &out, itemsPerConsumer, &wg)
    }

    wg.Wait()
}
