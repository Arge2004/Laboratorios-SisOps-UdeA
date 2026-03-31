package main

func countVowelsConsonants(data []byte, count []int) {
    for _, c := range string(data) {
        switch c {
        case 'a', 'A', 'á', 'Á':
            count[1]++
        case 'e', 'E', 'é', 'É':
            count[2]++
        case 'i', 'I', 'í', 'Í':
            count[3]++
        case 'o', 'O', 'ó', 'Ó':
            count[4]++
        case 'u', 'U', 'ú', 'Ú':
            count[5]++
        case ' ':
            continue 
        default:
            count[6]++
        }
    }

    count[0] = 0 // Aseguramos que empiece en cero
    for i := 1; i <= 5; i++ {
        count[0] += count[i]
    }
}