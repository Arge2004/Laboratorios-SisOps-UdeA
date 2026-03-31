void count_vowels_consonants(const char *input, int *count) {

    for (const char *p = input; *p; ) {
        // Detectar vocales acentuadas UTF-8 (2 bytes)
        if ((unsigned char)p[0] == 0xC3) {
            switch ((unsigned char)p[1]) {
                case 0xA1: case 0x81: // á Á
                    count[1]++; p += 2; continue;
                case 0xA9: case 0x89: // é É
                    count[2]++; p += 2; continue;
                case 0xAD: case 0x8D: // í Í
                    count[3]++; p += 2; continue;
                case 0xB3: case 0x93: // ó Ó
                    count[4]++; p += 2; continue;
                case 0xBA: case 0x9A: // ú Ú
                    count[5]++; p += 2; continue;
                default:
                    count[6]++; p += 2; continue;
            }
        }
        switch (*p) {
            case 'a': case 'A': count[1]++; break;
            case 'e': case 'E': count[2]++; break;
            case 'i': case 'I': count[3]++; break;
            case 'o': case 'O': count[4]++; break;
            case 'u': case 'U': count[5]++; break;
            case ' ': break;
            default: count[6]++; break;
        }
        p++;
    }

    for (int i = 1; i <= 5; i++) {
        count[0] += count[i];
    }
}