char *strdup(const char *src);
char **lines(const char *filename, int *num_lines_out);
char **split(char *str, char *delim);
void removeChars(char *str, char c);
