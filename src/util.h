// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTIL_H
#define UTIL_H

/**
 * @brief A portable implementation of strdup.
 * @param src The source string to duplicate.
 * @return A pointer to the newly allocated duplicate string, or NULL on failure.
 */
char *strdup(const char *src);

/**
 * @brief Removes all occurrences of a character 'c' from 'str' in-place.
 */
void removeChars(char *str, char c);

#endif // UTIL_H
