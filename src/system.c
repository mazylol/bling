// SPDX-License-Identifier: GPL-3.0-or-later

#include "system.h"
#include "file.h"
#include "util.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

struct os get_os() {
    struct os os = {
        .name = NULL,
        .version = NULL,
        .build_id = NULL,
    };

    char *name_ptr = NULL;
    char *version_ptr = NULL;
    char *build_id_ptr = NULL;

    int num_lines = 0;
    char **fileLines = lines("/etc/os-release", &num_lines);

    for (int i = 0; i < num_lines; i++) {
        char *line = fileLines[i];

        if (strncmp(line, "NAME=", 5) == 0) {
            name_ptr = line + 5;
        } else if (strncmp(line, "VERSION_ID=", 11) == 0) {
            version_ptr = line + 11;
        } else if (strncmp(line, "BUILD_ID=", 9) == 0) {
            build_id_ptr = line + 9;
        }
    }

    // Remove quotes in-place *before* copying
    if (name_ptr != NULL) {
        removeChars(name_ptr, '"');
        os.name = strdup(name_ptr);
    }

    if (version_ptr != NULL) {
        removeChars(version_ptr, '"');
        os.version = strdup(version_ptr);
    }

    if (build_id_ptr != NULL) {
        removeChars(build_id_ptr, '"');
        os.build_id = strdup(build_id_ptr);
    }

    free_string_array(fileLines);

    return os;
}

struct mem get_meminfo() {
    struct mem mem = {
        .max_memory = 0.0,
        .used_memory = 0.0,
    };

    double total_mem_kb = 0.0;
    double avail_mem_kb = 0.0;

    int num_lines = 0;
    char **fileLines = lines("/proc/meminfo", &num_lines);

    for (int i = 0; i < num_lines; i++) {
        char *line = fileLines[i];

        if (strncmp(line, "MemTotal:", 9) == 0) {
            char *value_str = line + 9;
            while (isspace((unsigned char)*value_str)) { // Skip whitespace
                value_str++;
            }
            total_mem_kb = strtod(value_str, NULL); // Converts "12345 kB" to 12345.0
        } else if (strncmp(line, "MemAvailable:", 13) == 0) {
            char *value_str = line + 13;
            while (isspace((unsigned char)*value_str)) { // Skip whitespace
                value_str++;
            }
            avail_mem_kb = strtod(value_str, NULL);
        }
    }

    mem.max_memory = total_mem_kb / 1024.0 / 1024.0; // KiB to GiB
    if (total_mem_kb > 0 && avail_mem_kb > 0) {
        mem.used_memory = (total_mem_kb - avail_mem_kb) / 1024.0 / 1024.0; // KiB to GiB
    }

    free_string_array(fileLines);

    return mem;
}

struct uptime get_uptime() {
    const unsigned long long SEC_PER_MIN = 60;
    const unsigned long long SEC_PER_HOUR = 3600;
    const unsigned long long SEC_PER_DAY = 86400;

    int num_lines = 0;
    char **fileLines = lines("/proc/uptime", &num_lines);

    struct uptime up = { 0, 0, 0, 0 };

    if (fileLines != NULL && num_lines > 0) {
        // strtoull stops at the first non-numeric char (the space)
        unsigned long long total_seconds = strtoull(fileLines[0], NULL, 10);

        up.days = total_seconds / SEC_PER_DAY;
        up.hours = (total_seconds % SEC_PER_DAY) / SEC_PER_HOUR;
        up.minutes = (total_seconds % SEC_PER_HOUR) / SEC_PER_MIN;
        up.seconds = total_seconds % SEC_PER_MIN;
    }

    free_string_array(fileLines);

    return up;
}

struct disk get_diskinfo() {
    struct disk d = {
        .total_memory_gb = 0.0,
        .used_memory_gb = 0.0,
    };
    struct statvfs disk_info;

    if (statvfs("/", &disk_info) == 0) {
        // Use 1024.0 for floating point division
        const double to_gib = 1024.0 * 1024.0 * 1024.0;
        d.total_memory_gb = (double)disk_info.f_blocks * disk_info.f_frsize / to_gib;
        d.used_memory_gb = (double)(disk_info.f_blocks - disk_info.f_bfree) * disk_info.f_frsize / to_gib;
    } else {
        perror("statvfs failed");
    }

    return d;
}

struct cpu get_cpu() {
    int num_lines = 0;
    char **fileLines = lines("/proc/cpuinfo", &num_lines);

    struct cpu cpu = {
        .name = NULL,
        .cores = 0,
    };

    int first_iter = 1;
    for (int i = 0; i < num_lines; i++) {
        if (strncmp(fileLines[i], "model name", 10) == 0) {
            cpu.cores++;

            if (first_iter) {
                char *colon = strchr(fileLines[i], ':');
                if (colon) {
                    cpu.name = strdup(colon + 2);
                }
                first_iter = 0;
            }
        }
    }

    free_string_array(fileLines);

    return cpu;
}

char *get_hostname() {
    int num_lines = 0;
    char **fileLines = lines("/etc/hostname", &num_lines);

    char *hostname = NULL;

    if (fileLines != NULL && num_lines > 0) {
        hostname = strdup(fileLines[0]);
    } else {
        hostname = strdup("unknown");
    }

    free_string_array(fileLines);

    return hostname;
}

char *get_kernel() {
    int num_lines = 0;
    char **fileLines = lines("/proc/version", &num_lines);

    char *kernel = NULL;

    if (fileLines != NULL && num_lines > 0) {
        char **kernel_parts = split(fileLines[0], " ");

        if (kernel_parts != NULL && kernel_parts[0] != NULL && kernel_parts[1] != NULL && kernel_parts[2] != NULL) {
            kernel = strdup(kernel_parts[2]);
        } else {
            kernel = strdup("unknown");
        }

        free_string_array(kernel_parts);
    } else {
        kernel = strdup("unknown");
    }

    free_string_array(fileLines);

    return kernel;
}
