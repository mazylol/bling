#include "util.h"

#include <malloc.h>
#include <string.h>

char *strdup(const char *src) {
    char *dst = malloc(strlen(src) + 1);
    if (dst == NULL)
        return NULL;
    strcpy(dst, src);
    return dst;
}

void removeChars(char *str, char c) {
    int i, j = 0;
    for (i = 0; str[i]; i++) {
        if (str[i] != c) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}
