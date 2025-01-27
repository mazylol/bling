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

char **split(char *str, char *delim) {
    char **result = NULL;
    size_t count = 0;
    char *token = strtok(str, delim);

    while (token) {
        char **temp = realloc(result, (count + 1) * sizeof(char *));
        if (temp == NULL) {
            perror("Memory allocation error");
            break;
        }
        result = temp;

        result[count] = token;
        count++;

        token = strtok(NULL, delim);
    }

    return result;
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
