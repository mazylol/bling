// SPDX-License-Identifier: GPL-3.0-or-later

#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8
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

    int capacity = INITIAL_CAPACITY;
    char **lines = malloc(capacity * sizeof(char *));
    if (lines == NULL) {
        perror("Memory allocation error");
        fclose(file);
        return NULL;
    }

    int num_lines = 0;
    char line[MAX_LINE_LENGTH];

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        // Resize if needed
        if (num_lines >= capacity - 1) { // -1 for the NULL terminator
            capacity *= 2;
            char **temp = realloc(lines, capacity * sizeof(char *));
            if (temp == NULL) {
                perror("Memory allocation error");
                // Cleanup on failure
                lines[num_lines] = NULL; // Make it valid for free_string_array
                free_string_array(lines);
                fclose(file);
                return NULL;
            }
            lines = temp;
        }

        // Strip newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        lines[num_lines] = strdup(line);
        if (lines[num_lines] == NULL) {
            perror("Memory allocation error");
            // Cleanup on strdup failure
            lines[num_lines] = NULL;
            free_string_array(lines);
            fclose(file);
            return NULL;
        }
        num_lines++;
    }

    if (ferror(file)) {
        // Handle read error
        free_string_array(lines); // 'lines' is NULL-terminated on failure
        fclose(file);
        return NULL;
    }

    fclose(file);

    if (num_lines_out != NULL) {
        *num_lines_out = num_lines;
    }

    lines[num_lines] = NULL; // Add the NULL terminator
    return lines;
}

char **split(const char *str, const char *delim) {
    // Create a writable copy of the string, as strtok modifies it
    char *str_copy = strdup(str);
    if (str_copy == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    int capacity = INITIAL_CAPACITY;
    char **result = malloc(capacity * sizeof(char *));
    if (result == NULL) {
        perror("Memory allocation error");
        free(str_copy);
        return NULL;
    }

    int count = 0;
    char *token = strtok(str_copy, delim);

    while (token) {
        // Resize if needed
        if (count >= capacity - 1) { // -1 for the NULL terminator
            capacity *= 2;
            char **temp = realloc(result, capacity * sizeof(char *));
            if (temp == NULL) {
                perror("Memory allocation error");
                // Cleanup
                result[count] = NULL;
                free_string_array(result);
                free(str_copy);
                return NULL;
            }
            result = temp;
        }

        // Copy the token
        result[count] = strdup(token);
        if (result[count] == NULL) {
            perror("Memory allocation error");
            // Cleanup
            result[count] = NULL;
            free_string_array(result);
            free(str_copy);
            return NULL;
        }
        count++;

        token = strtok(NULL, delim);
    }

    free(str_copy);       // Free the temporary copy
    result[count] = NULL; // Add the NULL terminator
    return result;
}

void free_string_array(char **array) {
    if (array == NULL) {
        return;
    }
    for (int i = 0; array[i] != NULL; i++) {
        free(array[i]); // Free each individual string
    }
    free(array); // Free the array of pointers
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