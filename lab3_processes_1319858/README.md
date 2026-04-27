# Laboratorio 3: Multiplicacion de matrices (C y Go)


## Integrantes

- Argenis Medina Morales - argenis.medina@udea.edu.co
- Juan Diego Duque Jimenez - juan.duque31@udea.edu.co

## Cómo ejecutar la versión en C

```bash
cd c
gcc -std=c11 -O2 -Wall -Wextra -pedantic -o parallel_matrix_multiply parallel_matrix_multiply.c parallel.c sequential.c
./parallel_matrix_multiply matrix_a.txt matrix_b.txt result.txt 5
```

## Cómo ejecutar la versión en Go

```bash
cd go
go build -o parallel_matrix_multiply parallel_matrix_multiply.go parallel.go sequential.go
./parallel_matrix_multiply matrix_a.txt matrix_b.txt result.txt 5
```

## Nota importante del reporte

Aunque el laboratorio tiene ambas versiones (C y Go), **el analisis de rendimiento que aparece abajo fue realizado con la version en C**.

## 1) Eleccion de IPC 

Se eligio memoria compartida System V (`shmget`, `shmat`, `shmctl`) junto con `fork()` por estas razones:

1. La carga es de alto volumen de datos (matrices completas), no de mensajes pequeños.
2. Cada hijo escribe su bloque de filas directamente en memoria compartida, sin serializar ni copiar por `pipe` o cola de mensajes.
3. El problema se divide por filas disjuntas, evitando colisiones de escritura.
4. La sincronizacion se resuelve de forma simple con `waitpid`.

### Trade-off

- Ventaja: bajo costo de transferencia de datos para este problema.
- Costo: manejo manual de memoria compartida y ciclo de vida de procesos.

## 2) Metodología del análisis (C)

1. Se compilo el programa en C.
2. Se ejecutaron pruebas con matrices cuadradas grandes: `N = 600, 900, 1200`.
3. Se evaluaron valores de `k` en `1 2 3 4 5 6 8 10 12` (solo los divisibles por cada `N`).
4. Cada configuracion se repitio `3` veces.
5. Se midieron:
   - `T_seq`: tiempo secuencial promedio
   - `T_par`: tiempo paralelo promedio
   - `Speedup = T_seq / T_par`
   - `Efficiency = Speedup / k`

Archivos generados por el benchmark (carpeta dedicada `benchmark/`):

- `benchmark/benchmark_raw.csv`
- `benchmark/benchmark_summary.csv`
- `benchmark/plots/time_vs_k.svg`
- `benchmark/plots/speedup_vs_k.svg`

## 3) Resultados (matrices grandes)

Fuente: `benchmark/benchmark_summary.csv`

| N | k | T_seq prom (s) | T_par prom (s) | Speedup | Efficiency |
|---:|---:|---:|---:|---:|---:|
| 600 | 1 | 0.1467 | 0.1497 | 0.98 | 0.98 |
| 600 | 2 | 0.1410 | 0.0917 | 1.54 | 0.77 |
| 600 | 3 | 0.1387 | 0.0617 | 2.25 | 0.75 |
| 600 | 4 | 0.1507 | 0.0593 | 2.54 | 0.63 |
| 600 | 5 | 0.1460 | 0.0567 | 2.58 | 0.52 |
| 600 | 6 | 0.1520 | 0.0460 | 3.30 | 0.55 |
| 600 | 8 | 0.1450 | 0.0457 | 3.18 | 0.40 |
| 600 | 10 | 0.1537 | 0.0477 | 3.22 | 0.32 |
| 600 | 12 | 0.1500 | 0.0460 | 3.26 | 0.27 |
| 900 | 1 | 0.7643 | 0.8323 | 0.92 | 0.92 |
| 900 | 2 | 0.7777 | 0.4320 | 1.80 | 0.90 |
| 900 | 3 | 0.7817 | 0.3330 | 2.35 | 0.78 |
| 900 | 4 | 0.7967 | 0.2613 | 3.05 | 0.76 |
| 900 | 5 | 0.7763 | 0.2497 | 3.11 | 0.62 |
| 900 | 6 | 0.7413 | 0.2407 | 3.08 | 0.51 |
| 900 | 10 | 0.7563 | 0.2947 | 2.57 | 0.26 |
| 900 | 12 | 0.7430 | 0.3153 | 2.36 | 0.20 |
| 1200 | 1 | 2.6877 | 2.7610 | 0.97 | 0.97 |
| 1200 | 2 | 2.6703 | 1.3473 | 1.98 | 0.99 |
| 1200 | 3 | 2.6887 | 1.0663 | 2.52 | 0.84 |
| 1200 | 4 | 2.6413 | 0.9727 | 2.72 | 0.68 |
| 1200 | 5 | 2.7163 | 1.0897 | 2.49 | 0.50 |
| 1200 | 6 | 2.6173 | 1.1090 | 2.36 | 0.39 |
| 1200 | 8 | 2.6417 | 1.5513 | 1.70 | 0.21 |
| 1200 | 10 | 2.6157 | 1.4477 | 1.81 | 0.18 |
| 1200 | 12 | 2.6390 | 1.4130 | 1.87 | 0.16 |

## 4) Gráficas

### Tiempo paralelo promedio vs k

![Tiempo paralelo promedio](benchmark/plots/time_vs_k.svg)

### Speedup vs k (incluye referencia ideal S(k)=k)

![Speedup vs k](benchmark/plots/speedup_vs_k.svg)

## 5) Interpretación

1. Para `k=1`, la version paralela suele ser mas lenta o similar por overhead de `fork + IPC + wait`.
2. Con matrices grandes aparece ganancia clara por paralelismo real.
3. El mejor speedup medido fue aproximadamente `3.30x` en `N=600, k=6`.
4. Al aumentar demasiado `k`, la eficiencia cae por costos de coordinación, cache y planificacion del SO.

## 6) Reproducibilidad

Para regenerar el análisis con matrices grandes:

```bash
cd benchmark
python3 benchmark.py --sizes 600 900 1200 --k-values 1 2 3 4 5 6 8 10 12 --repeats 3
python3 plot_svg.py
```

Esto recrea el CSV de resultados y las gráficas en `benchmark/plots/`.
