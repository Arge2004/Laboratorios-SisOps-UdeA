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

    data := []byte(os.Args[1])

    if len(data) > 100 {
        fmt.Println("Error: la cadena supera los 100 caracteres.")
        os.Exit(1)
    }
    
    if validCount := cleanValidate(data); validCount == 0 {
        fmt.Println("Error: la cadena no contiene caracteres válidos.")
        os.Exit(1)
    }
    
    reverseString(data)

    counts := make([]int, 7)
    countVowelsConsonants(data, counts)

    
    fmt.Printf("%s ", string(data))

    for i := 0; i < 6; i++ {
        if i == 0 || counts[i] > 0 { 
             fmt.Printf("%d ", counts[i])
        }
    }
    fmt.Printf("%d ", counts[6])


    changeSpacesToUnderscore(data)

    fmt.Printf("%s\n", string(data))
}