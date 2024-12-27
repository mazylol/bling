#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

#include "file.h"

struct lsb {
    char *distrib_id;
    char *distrib_release;
};

struct lsb parse_lsb(char **fileLines, int num_lines) {
    struct lsb lsb;
    lsb.distrib_id = NULL;
    lsb.distrib_release = NULL;

    for (int i = 0; i < num_lines; i++) {
        const char *line = fileLines[i];
        if (strstr(line, "DISTRIB_ID=") != NULL) {
            lsb.distrib_id = strdup(strstr(line, "=") + 1);
        } else if (strstr(line, "DISTRIB_RELEASE=") != NULL) {
            lsb.distrib_release = strdup(strstr(line, "=") + 1);
        }
    }

    return lsb;
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
            char *total_memory = strdup(strstr(line, ":") + 1);
            total_memory[strlen(total_memory) - 2] = '\0';

            // convert to GB
            double total_memory_gb = strtod(total_memory, NULL) / 1024 / 1024;
            mem.max_memory = total_memory_gb;
        } else if (strstr(line, "MemAvailable:") != NULL) {
            // remove the "kB" from the string
            char *available_memory = strdup(strstr(line, ":") + 1);
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

    struct lsb lsb = parse_lsb(lines("/etc/lsb-release", &num_lines), num_lines);

    char *proc_version = lines("/proc/version", &num_lines)[0];

    char *kernel = strtok(proc_version, " ");
    kernel = strtok(NULL, " ");
    kernel = strtok(NULL, " ");

    char *shell = strtok(getenv("SHELL"), "/");
    shell = strtok(NULL, "/");
    shell = strtok(NULL, "/");

    char **fileLines = lines("/proc/meminfo", &num_lines);
    struct mem mem = parse_meminfo(fileLines, num_lines);

    unsigned long total_memory_gb = 0;
    unsigned long used_memory_gb = 0;
    struct statvfs disk_info;
    if (statvfs("/", &disk_info) == 0) {
        total_memory_gb = disk_info.f_blocks * disk_info.f_frsize / 1024 / 1024 / 1024;
        used_memory_gb = (disk_info.f_blocks - disk_info.f_bfree) * disk_info.f_frsize / 1024 / 1024 / 1024;
    }

    printf("\033[1muser/host\033[0m %s@%s\n", username, hostname);
    if (lsb.distrib_release != NULL) {
        printf("\033[1mdistro\033[0m    %s %s\n", lsb.distrib_id, lsb.distrib_release);
    } else {
        printf("\033[1mdistro\033[0m    %s\n", lsb.distrib_id);
    }
    printf("\033[1mkernel\033[0m    %s\n", kernel);
    printf("\033[1mshell\033[0m     %s\n", shell);
    printf("\033[1mram\033[0m       %.1f / %.1f GiB\n", mem.used_memory, mem.max_memory);
    printf("\033[1muptime\033[0m    %lu minutes\n", parse_uptime(lines("/proc/uptime", &num_lines)[0]));
    printf("\033[1mdisk\033[0m      %.1lu / %.1lu GiB\n", used_memory_gb, total_memory_gb);

    return 0;
}
