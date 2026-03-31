void reverse_string(char *input) {
int len = strlen(input);
    char *lastLetter = input + len - 1; // Puntero al final
    char *firstLetter = input;          // Puntero al inicio

    while (firstLetter < lastLetter) {
        char temp = *firstLetter;   
        *firstLetter = *lastLetter; 
        *lastLetter = temp;         

        firstLetter++;
        lastLetter--;
    }

    for (int i = 0; i < len - 1; i++) {
        if ((unsigned char)input[i] >= 0x80 && (unsigned char)input[i] <= 0xBF) {
            unsigned char prev = (unsigned char)input[i+1];
            if (prev >= 0xC2 && prev <= 0xDF) {
                char temp = input[i];
                input[i] = input[i+1];
                input[i+1] = temp;
                i++; 
            }
        }
    }
}