# Lab 4 - Synchronization Mechanisms

## Equipo:

- Argenis Medina Morales
- Juan Diego Duque

## Archivos

- `Lab4/go/queue.go`
- `Lab4/go/producer_consumer.go`
- `Lab4/go/dining_philosophers.go`
- `Lab4/c/queue.c`
- `Lab4/c/producer_consumer.c`
- `Lab4/c/dining_philosophers.c`

## Enfoque 
Para evitar deadlock en el problema de los filosofos se usa un semaforo
global llamado `room` con capacidad de 4. Esto limita a 4 filosofos
intentando comer al mismo tiempo. Con eso se elimina el ciclo de espera
de los 5 filosofos (nunca estan los 5 tomando un tenedor y esperando el
otro).

Cada filosofo toma sus dos tenedores (mutex) cuando obtiene permiso del
semaforo, come y luego libera ambos tenedores y devuelve el permiso al
semaforo. Asi siempre hay progreso y se evita el deadlock.

## Compilacion y ejecucion (C)

```bash
gcc Lab4/c/queue.c -pthread -o queue
gcc Lab4/c/producer_consumer.c -pthread -o producer_consumer
gcc Lab4/c/dining_philosophers.c -pthread -o dining_philosophers

./queue
./producer_consumer
./dining_philosophers
```

## Ejecucion (Go)

```bash
go run Lab4/go/queue.go
go run Lab4/go/producer_consumer.go
go run Lab4/go/dining_philosophers.go
```

## Salidas de ejemplo

### Filosofos (Go o C)

```
Filosofo 1 piensa
Filosofo 2 piensa
Filosofo 3 piensa
Filosofo 4 piensa
Filosofo 5 piensa
Filosofo 1 toma tenedor 1
Filosofo 2 toma tenedor 2
Filosofo 2 toma tenedor 3
Filosofo 2 come (1)
Filosofo 4 toma tenedor 4
Filosofo 4 toma tenedor 5
Filosofo 4 come (1)
Filosofo 2 deja tenedor 3
Filosofo 2 deja tenedor 2
Filosofo 3 toma tenedor 3
Filosofo 2 piensa
Filosofo 1 toma tenedor 2
Filosofo 1 come (1)
Filosofo 4 deja tenedor 5
Filosofo 4 deja tenedor 4
Filosofo 4 piensa
Filosofo 5 toma tenedor 5
Filosofo 3 toma tenedor 4
Filosofo 3 come (1)
Filosofo 1 deja tenedor 2
Filosofo 3 deja tenedor 4
Filosofo 3 deja tenedor 3
Filosofo 3 piensa
Filosofo 4 toma tenedor 4
Filosofo 1 deja tenedor 1
Filosofo 1 piensa
Filosofo 2 toma tenedor 2
Filosofo 2 toma tenedor 3
Filosofo 2 come (2)
Filosofo 5 toma tenedor 1
Filosofo 5 come (1)
Filosofo 5 deja tenedor 1
Filosofo 2 deja tenedor 3
Filosofo 2 deja tenedor 2
Filosofo 3 toma tenedor 3
Filosofo 5 deja tenedor 5
Filosofo 5 piensa
Filosofo 2 piensa
Filosofo 4 toma tenedor 5
Filosofo 1 toma tenedor 1
Filosofo 1 toma tenedor 2
Filosofo 1 come (2)
Filosofo 4 come (2)
Filosofo 1 deja tenedor 2
Filosofo 1 deja tenedor 1
Filosofo 2 toma tenedor 2
Filosofo 4 deja tenedor 5
Filosofo 1 piensa
Filosofo 5 toma tenedor 5
Filosofo 5 toma tenedor 1
Filosofo 5 come (2)
Filosofo 4 deja tenedor 4
Filosofo 4 piensa
Filosofo 3 toma tenedor 4
Filosofo 3 come (2)
Filosofo 5 deja tenedor 1
Filosofo 5 deja tenedor 5
Filosofo 5 piensa
Filosofo 1 toma tenedor 1
Filosofo 3 deja tenedor 4
Filosofo 4 toma tenedor 4
Filosofo 4 toma tenedor 5
Filosofo 4 come (3)
Filosofo 3 deja tenedor 3
Filosofo 3 piensa
Filosofo 2 toma tenedor 3
Filosofo 2 come (3)
Filosofo 4 deja tenedor 5
Filosofo 4 deja tenedor 4
Filosofo 5 toma tenedor 5
Filosofo 2 deja tenedor 3
Filosofo 3 toma tenedor 3
Filosofo 3 toma tenedor 4
Filosofo 3 come (3)
Filosofo 2 deja tenedor 2
Filosofo 1 toma tenedor 2
Filosofo 1 come (3)
Filosofo 3 deja tenedor 4
Filosofo 3 deja tenedor 3
Filosofo 1 deja tenedor 2
Filosofo 1 deja tenedor 1
Filosofo 5 toma tenedor 1
Filosofo 5 come (3)
Filosofo 5 deja tenedor 1
Filosofo 5 deja tenedor 5
```
