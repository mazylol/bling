#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

char *strdup(const char *src) {
    char *dst = malloc(strlen(src) + 1);
    if (dst == NULL)
        return NULL;
    strcpy(dst, src);
    return dst;
}

char **lines(const char *filename, int *num_lines_out) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    char **lines = NULL;
    int num_lines = 0;
    char line[MAX_LINE_LENGTH];

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        char **temp = realloc(lines, (num_lines + 1) * sizeof(char *));
        if (temp == NULL) {
            perror("Memory allocation error");
            break;
        }
        lines = temp;

        lines[num_lines] = strdup(line);

        num_lines++;
    }

    if (ferror(file)) {
        for (int i = 0; i < num_lines; i++) {
            free(lines[i]);
        }
        free(lines);
        return NULL;
    }

    fclose(file);

    if (num_lines_out != NULL) {
        *num_lines_out = num_lines;
    }

    return lines;
}
