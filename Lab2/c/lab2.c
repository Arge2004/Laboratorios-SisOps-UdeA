#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
    
        if (argc != 2) {
    		printf("Se debe usar: %s <cadena>\n", argv[0]);
    		return 1;
    	}
    
        if (strlen(argv[1]) > 100) {
    		printf("Error: la cadena supera los 100 caracteres.\n");
    		return 1;
    	}
    
        if (!clean_validate(argv[1])) {
            printf("Error: la cadena no contiene caracteres válidos.\n");
            return 1;
    
        char palabra[100];
        char output[150];
        char *out_ptr = output;
        strcpy(palabra, argv[1]);
        int size = strlen(palabra);
        char *ptr = palabra + size;

        for (int i=0; i <= size; i++) {
                palabra[size+i] = palabra[size-i-1];
        }
        strcpy(palabra, ptr);
        out_ptr += sprintf(out_ptr, "%s ", palabra);
        palabra[ size ]='\0';
        
        printf("Reverse String: %s \n", palabra);
        int totalVowels = 0;
        int vowels[5] = {0};
        int consonants = 0;

        ptr = palabra;
        while (*ptr != '\0') {
                char p = tolower(*ptr);

                switch (tolower(*ptr)) {
                        case 'a':
                                vowels[0]++;
                                totalVowels++;
                                break;
                        case 'e':
                                vowels[1]++;
                                totalVowels++;
                                break;
                        case 'i':
                                vowels[2]++;
                                totalVowels++;
                                break;
                        case 'o':
                                vowels[3]++;
                                totalVowels++;
                                break;
                        case 'u':
                                vowels[4]++;
                                totalVowels++;
                                break;
                        case ' ':
                                *ptr='_';
                                break;
                        default:
                                consonants++;
                }
                ptr++;
        }

        printf("Number of vowels: %d \n", totalVowels);
        out_ptr += sprintf(out_ptr, "%d ", totalVowels);

        for (int i=0; i<5; i++) {
                if (vowels[i]!=0) {
                        printf("Number of a: %d, ", vowels[i]);
                        out_ptr += sprintf(out_ptr, "%d ", vowels[i]);
                }
        }

        printf("\nNumber of consonants: %d \n", consonants);
        out_ptr += sprintf(out_ptr, "%d ", consonants);

        printf("Modified_String: %s \n", palabra);
        out_ptr += sprintf(out_ptr, "%s", palabra);
        printf(output);

        return 0;
}
