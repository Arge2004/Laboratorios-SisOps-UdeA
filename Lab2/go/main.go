package main

import (
	"fmt"
	"os"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Se debe usar: %s <cadena>\n", os.Args[0])
		os.Exit(1)
	}
	input := os.Args[1]
	if len(input) > 100 {
		fmt.Println("Error: la cadena supera los 100 caracteres.")
		os.Exit(1)
	}
	if !cleanValidate(&input) {
		fmt.Println("Error: la cadena no contiene caracteres válidos.")
		os.Exit(1)
	}
	reverseString(&input)
	count := make([]int, 7)
	countVowelsConsonants(input, count)
	fmt.Printf("%s %d ", input, count[0])
	for i := 1; i <= 5; i++ {
		if count[i] > 0 {
			fmt.Printf("%d ", count[i])
		}
	}
	fmt.Printf("%d ", count[6])
	changeSpacesToUnderscore(&input)
	fmt.Printf("%s\n", input)
}
