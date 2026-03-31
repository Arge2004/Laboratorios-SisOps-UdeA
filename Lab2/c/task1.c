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
}