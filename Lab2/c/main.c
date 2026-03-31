#include <stdio.h>
#include <string.h>
#include "task1.c"
#include "task2.c"
#include "task3.c"
#include "validations.c"

int main(int argc, char *argv[]) {
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
    }
	
    reverse_string(argv[1]);
    int count[7] = {0};
    count_vowels_consonants(argv[1], count);
    printf("%s %d ", argv[1], count[0]);
    for (int i = 1; i <= 5; i++) {
        if (count[i] > 0) {
            printf("%d ", count[i]);
        }
    }
    printf("%d ", count[6]);
    change_spaces_to_underscore(argv[1]);
    printf("%s\n", argv[1]);
	return 0;
}
