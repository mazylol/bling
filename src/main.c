#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

#include "file.h"

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

struct os {
    char *name;
    char *version;
    char *build_id;
};

struct os parse_os(char **fileLines, int num_lines) {
    struct os os;
    os.name = NULL;
    os.version = NULL;
    os.build_id = NULL;

    for (int i = 0; i < num_lines; i++) {
        char *line = fileLines[i];

        if (strncmp(line, "NAME=", 5) == 0) {
            os.name = line + 5;
        } else if (strncmp(line, "VERSION_ID=", 11) == 0) {
            os.version = line + 11;
        } else if (strncmp(line, "BUILD_ID=", 9) == 0) {
            os.build_id = line + 9;
        }
    }

    return os;
}

struct mem {
    double max_memory;
    double used_memory;
};

void removeSpaces(char *str) {
    int i, j = 0;
    for (i = 0; str[i]; i++) {
        if (str[i] != ' ') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

struct mem parse_meminfo(char **fileLines, int num_lines) {
    struct mem mem = {0, 0};

    for (int i = 0; i < num_lines; i++) {
        char *line = fileLines[i];
        removeSpaces(line);

        if (strncmp(line, "MemTotal:", 9) == 0) {
            char *total_memory = strstr(line, ":") + 1;
            total_memory[strlen(total_memory) - 2] = '\0';

            double total_memory_gb = strtod(total_memory, NULL) / 1024 / 1024;
            mem.max_memory = total_memory_gb;
        } else if (strncmp(line, "MemAvailable:", 13) == 0) {
            char *available_memory = strstr(line, ":") + 1;
            available_memory[strlen(available_memory) - 2] = '\0';

            double available_memory_gb = strtod(available_memory, NULL) / 1024 / 1024;
            mem.used_memory = mem.max_memory - available_memory_gb;
        }
    }

    return mem;
}

size_t parse_uptime(char *uptime) {
    size_t minutes = 0;

    size_t seconds = strtoull(strtok(uptime, " "), NULL, 10);
    minutes = seconds / 60;

    return minutes;
}

int main() {
    const char *username = getenv("USER");

    int num_lines = 0;
    const char *hostname = lines("/etc/hostname", &num_lines)[0];

    char **fileLines = lines("/etc/os-release", &num_lines);
    struct os os = parse_os(fileLines, num_lines);

    char *proc_version = lines("/proc/version", &num_lines)[0];

    char *kernel = split(proc_version, " ")[2];

    char *shell = strrchr(getenv("SHELL"), '/') + 1;

    fileLines = lines("/proc/meminfo", &num_lines);
    struct mem mem = parse_meminfo(fileLines, num_lines);

    unsigned long total_memory_gb = 0;
    unsigned long used_memory_gb = 0;
    struct statvfs disk_info;
    if (statvfs("/", &disk_info) == 0) {
        total_memory_gb = disk_info.f_blocks * disk_info.f_frsize / 1024 / 1024 / 1024;
        used_memory_gb = (disk_info.f_blocks - disk_info.f_bfree) * disk_info.f_frsize / 1024 / 1024 / 1024;
    }

    printf("\033[1muser/host\033[0m %s@%s\n", username, hostname);

    printf("\033[1mos\033[0m        %s", os.name);
    if (os.version != NULL) {
        printf(" %s", os.version);
    }
    if (os.build_id != NULL) {
        printf(" (%s)", os.build_id);
    }
    printf("\n");

    printf("\033[1mkernel\033[0m    %s\n", kernel);
    printf("\033[1mshell\033[0m     %s\n", shell);
    printf("\033[1mram\033[0m       %.1f / %.1f GiB\n", mem.used_memory, mem.max_memory);
    printf("\033[1muptime\033[0m    %lu minutes\n", parse_uptime(lines("/proc/uptime", &num_lines)[0]));
    printf("\033[1mdisk\033[0m      %.1lu / %.1lu GiB\n", used_memory_gb, total_memory_gb);

    return 0;
}
