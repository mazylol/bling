#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

#include "colors.h"
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
        char *line = fileLines[i];

        if (strncmp(line, "NAME=", 5) == 0) {
            os.name = line + 5;
        } else if (strncmp(line, "VERSION_ID=", 11) == 0) {
            os.version = line + 11;
        } else if (strncmp(line, "BUILD_ID=", 9) == 0) {
            os.build_id = line + 9;
        }
    }

    if (os.name != NULL) {
        removeChars(os.name, '"');
    }

    if (os.version != NULL) {
        removeChars(os.version, '"');
    }

    if (os.build_id != NULL) {
        removeChars(os.build_id, '"');
    }

    return os;
}

struct mem {
    double max_memory;
    double used_memory;
};

struct mem parse_meminfo(char **fileLines, int num_lines) {
    struct mem mem = {0, 0};

    for (int i = 0; i < num_lines; i++) {
        char *line = fileLines[i];
        removeChars(line, ' ');

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

    printf("%suser/host%s %s@%s\n", BHGRN, CRESET, username, hostname);

    printf("%sos%s        %s", BHCYN, CRESET, os.name);
    if (os.version != NULL) {
        printf(" %s", os.version);
    }
    if (os.build_id != NULL) {
        printf(" (%s)", os.build_id);
    }
    printf("\n");

    printf("%skernel%s    %s\n", BHYEL, CRESET, kernel);
    printf("%sshell%s     %s\n", BHMAG, CRESET, shell);
    printf("%sram%s       %.1f / %.1f GiB\n", BHBLU, CRESET, mem.used_memory, mem.max_memory);
    printf("%suptime%s    %lu minutes\n", BHBLK, CRESET, parse_uptime(lines("/proc/uptime", &num_lines)[0]));
    printf("%sdisk%s      %.1lu / %.1lu GiB\n", BHRED, CRESET, used_memory_gb, total_memory_gb);

    return 0;
}
