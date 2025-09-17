// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

#include "LICENSE.h"
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

struct uptime {
    size_t seconds;
    size_t minutes;
    size_t hours;
    size_t days;
};

struct uptime parse_uptime(char *uptime) {
    struct uptime up = {};

    unsigned long long total_seconds = strtoull(strtok(uptime, " "), NULL, 10);

    up.days = total_seconds / 86400;
    up.hours = (total_seconds % 86400) / 3600;
    up.minutes = (total_seconds % 3600) / 60;
    up.seconds = total_seconds % 60;

    return up;
}

const char *helpString = "bling, a very simple system info tool, kind of like neofetch but worse\n\n--help: this screen\n--license: view the license\n";

int main(int argc, char **argv) {
    if (argc != 1) {
        if (strcmp(argv[1], "--help") == 0) {
            printf("bling, a very simple system info tool, kind of like neofetch but worse\n\n--help: this screen\n--license: view the license\n");
            return 0;
        } else if (strcmp(argv[1], "--license") == 0) {
            printf("%s", LICENSE);
            return 0;
        } else if (strstr(argv[1], "--") != NULL) {
            printf("%s", helpString);
            return 0;
        }
    }

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

    struct uptime uptime = parse_uptime(lines("/proc/uptime", &num_lines)[0]);

    printf("%skernel%s    %s\n", BHYEL, CRESET, kernel);
    printf("%sshell%s     %s\n", BHMAG, CRESET, shell);
    printf("%sram%s       %.1f / %.1f GiB\n", BHBLU, CRESET, mem.used_memory, mem.max_memory);
    printf("%suptime%s    %lud %luh %lum %lus\n", BHBLK, CRESET, uptime.days, uptime.hours, uptime.minutes, uptime.seconds);
    printf("%sdisk%s      %.1lu / %.1lu GiB\n", BHRED, CRESET, used_memory_gb, total_memory_gb);

    return 0;
}
