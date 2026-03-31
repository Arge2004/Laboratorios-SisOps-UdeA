void change_spaces_to_underscore(char *input) {
    for (char *p = input; *p; p++) {
        if (*p == ' ') {
            *p = '_';
        }
    }
}