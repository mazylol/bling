#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

#include "file.h"

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
        const char *line = fileLines[i];

        if (strstr(line, "NAME=") != NULL) {
            os.name = strstr(line, "=") + 1;
        } else if (strstr(line, "VERSION_ID=") != NULL) {
            os.version = strstr(line, "=") + 1;
        } else if (strstr(line, "BUILD_ID=") != NULL) {
            os.build_id = strstr(line, "=") + 1;
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

        if (strstr(line, "MemTotal:") != NULL) {
            // remove the "kB" from the string
            char *total_memory = strstr(line, ":") + 1;
            total_memory[strlen(total_memory) - 2] = '\0';

            // convert to GB
            double total_memory_gb = strtod(total_memory, NULL) / 1024 / 1024;
            mem.max_memory = total_memory_gb;
        } else if (strstr(line, "MemAvailable:") != NULL) {
            // remove the "kB" from the string
            char *available_memory = strstr(line, ":") + 1;
            available_memory[strlen(available_memory) - 2] = '\0';

            // convert to GB
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

    char *kernel = strtok(proc_version, " ");
    kernel = strtok(NULL, " ");
    kernel = strtok(NULL, " ");

    char *shell = strtok(getenv("SHELL"), "/");
    shell = strtok(NULL, "/");
    shell = strtok(NULL, "/");

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
