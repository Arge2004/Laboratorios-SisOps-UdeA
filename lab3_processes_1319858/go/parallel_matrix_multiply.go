package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
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
	if len(a)%k != 0 {
		fmt.Println("Error: el numero de filas de A debe ser divisible por k")
		os.Exit(1)
	}

	rows := len(a)
	cols := len(b[0])

	seqResult := makeMatrix(rows, cols)
	parResult := makeMatrix(rows, cols)

	startSeq := time.Now()
	multiplyMatrixSequential(a, b, seqResult)
	seqDuration := time.Since(startSeq)

	startPar := time.Now()
	multiplyMatrixParallel(a, b, k, parResult)
	parDuration := time.Since(startPar)

	if !equalMatrices(seqResult, parResult) {
		fmt.Println("Error: resultado paralelo no coincide con el secuencial")
		os.Exit(1)
	}

	if err := writeMatrixToFile(outputPath, parResult); err != nil {
		fmt.Printf("Error guardando resultado: %v\n", err)
		os.Exit(1)
	}

	seqSeconds := seqDuration.Seconds()
	parSeconds := parDuration.Seconds()
	speedup := 0.0
	if parSeconds > 0 {
		speedup = seqSeconds / parSeconds
	}

	fmt.Printf("Sequential time: %.3f seconds\n", seqSeconds)
	fmt.Printf("Parallel time (%d processes): %.3f seconds\n", k, parSeconds)
	fmt.Printf("Speedup: %.2fx\n", speedup)
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

func equalMatrices(a, b [][]int) bool {
	if len(a) != len(b) || len(a[0]) != len(b[0]) {
		return false
	}
	for i := 0; i < len(a); i++ {
		for j := 0; j < len(a[0]); j++ {
			if a[i][j] != b[i][j] {
				return false
			}
		}
	}
	return true
}
