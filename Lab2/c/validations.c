#include <ctype.h>

int clean_validate(char *input) {
    int i = 0; // Lector
    int j = 0; // Escritor

    while (input[i] != '\0') {
        if (isalpha((unsigned char)input[i]) || isspace((unsigned char)input[i])) {
            input[j] = input[i];
            j++;
        }
        i++;
    }
    
    input[j] = '\0'; 
    return (j > 0); 
}