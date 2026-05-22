package main

import (
    "fmt"
    "sync"
    "time"
)

func philosopher(id int, leftIdx int, rightIdx int, forks []sync.Mutex, room chan struct{}, meals int, wg *sync.WaitGroup) {
    defer wg.Done()
    left := &forks[leftIdx]
    right := &forks[rightIdx]
    for i := 0; i < meals; i++ {
        fmt.Printf("Filosofo %d piensa\n", id)
        time.Sleep(50 * time.Millisecond)

        <-room
        left.Lock()
        fmt.Printf("Filosofo %d toma tenedor %d\n", id, leftIdx+1)
        right.Lock()
        fmt.Printf("Filosofo %d toma tenedor %d\n", id, rightIdx+1)

        fmt.Printf("Filosofo %d come (%d)\n", id, i+1)
        time.Sleep(50 * time.Millisecond)

        right.Unlock()
        fmt.Printf("Filosofo %d deja tenedor %d\n", id, rightIdx+1)
        left.Unlock()
        fmt.Printf("Filosofo %d deja tenedor %d\n", id, leftIdx+1)
        room <- struct{}{}
    }
}

func main() {
    philosophers := 5
    meals := 3

    forks := make([]sync.Mutex, philosophers)
    room := make(chan struct{}, philosophers-1)
    for i := 0; i < philosophers-1; i++ {
        room <- struct{}{}
    }

    var wg sync.WaitGroup
    for i := 0; i < philosophers; i++ {
        wg.Add(1)
        leftIdx := i
        rightIdx := (i + 1) % philosophers
        go philosopher(i+1, leftIdx, rightIdx, forks, room, meals, &wg)
    }

    wg.Wait()
}
