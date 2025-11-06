// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @brief A portable implementation of strdup.
 * @param src The source string to duplicate.
 * @return A pointer to the newly allocated duplicate string, or NULL on failure.
 */
char *strdup(const char *src);
/**
 * @brief Reads all lines from a file into a NULL-terminated array of strings.
 *
 * @param filename The name of the file to read.
 * @param num_lines_out (Optional) A pointer to an int to store the number of lines read.
 * @return A NULL-terminated array of strings. The caller must free this
 * using free_string_array(). Returns NULL on failure.
 */
char **lines(const char *filename, int *num_lines_out);
/**
 * @brief Splits a string by a delimiter into a NULL-terminated array of strings.
 *
 * @param str The string to split.
 * @param delim The delimiter string.
 * @return A NULL-terminated array of strings. The caller must free this
 * using free_string_array(). Returns NULL on failure.
 */
char **split(const char *str, const char *delim);
/**
 * @brief Removes all occurrences of a character 'c' from 'str' in-place.
 */
void removeChars(char *str, char c);
/**
 * @brief Frees a NULL-terminated array of strings (e.g., from lines() or split()).
 */
void free_string_array(char **array);
