package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strconv"
	"strings"
	"syscall"
	"time"
	"unsafe"

	"github.com/gen2brain/shm"
)

func main() {
	if len(os.Args) != 5 {
		fmt.Printf("Uso: %s <matrizA.txt> <matrizB.txt> <salida.txt> <k>\n", os.Args[0])
		os.Exit(1)
	}

	matrixAPath := os.Args[1]
	matrixBPath := os.Args[2]
	outputPath := os.Args[3]

	k, err := strconv.Atoi(os.Args[4])
	if err != nil || k <= 0 {
		fmt.Println("Error: k debe ser un entero positivo")
		os.Exit(1)
	}

	a, err := readMatrixFromFile(matrixAPath)
	if err != nil {
		fmt.Printf("Error leyendo matriz A: %v\n", err)
		os.Exit(1)
	}

	b, err := readMatrixFromFile(matrixBPath)
	if err != nil {
		fmt.Printf("Error leyendo matriz B: %v\n", err)
		os.Exit(1)
	}

	if len(a) == 0 || len(b) == 0 {
		fmt.Println("Error: las matrices no pueden estar vacias")
		os.Exit(1)
	}
	if len(a[0]) != len(b) {
		fmt.Println("Error: dimensiones incompatibles (columnas de A != filas de B)")
		os.Exit(1)
	}
	if k > 1 && len(a)%k != 0 {
		fmt.Println("Error: el numero de filas de A debe ser divisible por k")
		os.Exit(1)
	}

	rows := len(a)
	cols := len(b[0])

	result := makeMatrix(rows, cols)

	start := time.Now()
	if k == 1 {
		multiplyMatrixSequential(a, b, result)
	} else {
		multiplyMatrixParallel(a, b, k, result)
	}
	duration := time.Since(start)

	if err := writeMatrixToFile(outputPath, result); err != nil {
		fmt.Printf("Error guardando resultado: %v\n", err)
		os.Exit(1)
	}

	mode := "parallel"
	if k == 1 {
		mode = "sequential"
	}
	fmt.Printf("Mode: %s\n", mode)
	fmt.Printf("Time (%d processes): %.3f seconds\n", k, duration.Seconds())
	fmt.Printf("Result saved in: %s\n", outputPath)
}

func readMatrixFromFile(path string) ([][]int, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)

	matrix := make([][]int, 0)
	cols := -1
	lineNo := 0
	for scanner.Scan() {
		lineNo++
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		parts := strings.Fields(line)
		if cols == -1 {
			cols = len(parts)
			if cols == 0 {
				continue
			}
		} else if len(parts) != cols {
			return nil, fmt.Errorf("fila invalida en linea %d: se esperaban %d valores", lineNo, cols)
		}

		row := make([]int, cols)
		for j := 0; j < cols; j++ {
			val, err := strconv.Atoi(parts[j])
			if err != nil {
				return nil, fmt.Errorf("valor no numerico en linea %d, columna %d", lineNo, j+1)
			}
			row[j] = val
		}
		matrix = append(matrix, row)
	}

	if err := scanner.Err(); err != nil {
		return nil, err
	}
	if len(matrix) == 0 {
		return nil, errors.New("archivo vacio")
	}

	return matrix, nil
}

func writeMatrixToFile(path string, matrix [][]int) error {
	file, err := os.Create(path)
	if err != nil {
		return err
	}
	defer file.Close()

	writer := bufio.NewWriter(file)
	for i := 0; i < len(matrix); i++ {
		for j := 0; j < len(matrix[i]); j++ {
			if j > 0 {
				if _, err := writer.WriteString(" "); err != nil {
					return err
				}
			}
			if _, err := fmt.Fprintf(writer, "%d", matrix[i][j]); err != nil {
				return err
			}
		}
		if _, err := writer.WriteString("\n"); err != nil {
			return err
		}
	}

	return writer.Flush()
}

func makeMatrix(rows, cols int) [][]int {
	matrix := make([][]int, rows)
	for i := 0; i < rows; i++ {
		matrix[i] = make([]int, cols)
	}
	return matrix
}

func multiplyMatrixSequential(a, b, c [][]int) {
	for i := 0; i < len(a); i++ {
		for j := 0; j < len(b[0]); j++ {
			for k := 0; k < len(b); k++ {
				c[i][j] += a[i][k] * b[k][j]
			}
		}
	}
}

func multiplyMatrixParallel(a, b [][]int, k int, c [][]int) {
	rows := len(a)
	if rows == 0 || len(b) == 0 || len(b[0]) == 0 || k <= 0 {
		return
	}

	cols := len(b[0])
	inner := len(b)
	if k > rows {
		k = rows
	}
	if rows%k != 0 {
		panic("rows must be divisible by k")
	}

	total := rows * cols
	bytesToShare := total * int(unsafe.Sizeof(int(0)))
	shmID, err := shm.Get(shm.IPC_PRIVATE, bytesToShare, shm.IPC_CREAT|0o600)
	if err != nil {
		panic(err)
	}
	defer shm.Ctl(shmID, shm.IPC_RMID, nil)

	mem, err := shm.At(shmID, 0, 0)
	if err != nil {
		panic(err)
	}
	defer shm.Dt(mem)

	if len(mem) < bytesToShare {
		panic("shared memory segment is smaller than expected")
	}

	shared := unsafe.Slice((*int)(unsafe.Pointer(&mem[0])), total)

	base := rows / k

	start := 0
	pids := make([]int, 0, k)
	for p := 0; p < k; p++ {
		chunk := base
		end := start + chunk

		pid, _, errno := syscall.RawSyscall(syscall.SYS_FORK, 0, 0, 0)
		if errno != 0 {
			panic(errno)
		}

		if pid == 0 {
			for i := start; i < end; i++ {
				for j := 0; j < cols; j++ {
					sum := 0
					for col := 0; col < inner; col++ {
						sum += a[i][col] * b[col][j]
					}
					shared[i*cols+j] = sum
				}
			}
			syscall.RawSyscall(syscall.SYS_EXIT, 0, 0, 0)
		}

		pids = append(pids, int(pid))

		start = end
	}

	for _, pid := range pids {
		var status syscall.WaitStatus
		_, err := syscall.Wait4(pid, &status, 0, nil)
		if err != nil {
			panic(err)
		}
	}

	for i := 0; i < rows; i++ {
		for j := 0; j < cols; j++ {
			c[i][j] = shared[i*cols+j]
		}
	}
}
