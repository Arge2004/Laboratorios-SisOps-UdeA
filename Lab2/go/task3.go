package main

func changeSpacesToUnderscore(data []byte) {
    for i := 0; i < len(data); i++ {
        if data[i] == ' ' {
            data[i] = '_'
        }
    }
}