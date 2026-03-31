package main

func cleanValidate(data []byte) int {
    j := 0
    for i := 0; i < len(data); i++ {
        c := data[i]
        if (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ' || c > 127 {
            data[j] = data[i]
            j++
        }
    }
    return j
}