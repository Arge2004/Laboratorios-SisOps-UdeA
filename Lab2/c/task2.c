void count_vowels_consonants(const char *input, int *count) {
    for (const char *p = input; *p; p++) {
        switch (*p) {
            case 'a': case 'A': count[1]++; break;
            case 'e': case 'E': count[2]++; break;
            case 'i': case 'I': count[3]++; break;
            case 'o': case 'O': count[4]++; break;
            case 'u': case 'U': count[5]++; break;
            case ' ': break; 
            default: count[6]++; break;
        }
    }

    for (int i = 1; i <= 5; i++) {
        count[0] += count[i];
    }
}