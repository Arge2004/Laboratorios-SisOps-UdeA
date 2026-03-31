package main

func reverseString(data []byte) {
    i := 0
    j := len(data) - 1
    for i < j {
        data[i], data[j] = data[j], data[i]
        i++
        j--
    }

	for k := 0; k < len(data)-1; k++ {
        if data[k] >= 0x80 && data[k] <= 0xBF {
            if k+1 < len(data) && data[k+1] >= 0xC2 && data[k+1] <= 0xDF {
                data[k], data[k+1] = data[k+1], data[k]
                k++ 
            }
        }
    }
}