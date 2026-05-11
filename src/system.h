// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_H
#define SYSTEM_H

#include <stddef.h>

struct os {
    char *name;
    char *version;
    char *build_id;
};

/**
 * @brief Parses /etc/os-release lines into an os struct.
 *
 * NOTE: This function now uses strdup to safely copy strings.
 * The caller is responsible for freeing os.name, os.version, and os.build_id.
 */
struct os get_os();

struct mem {
    double max_memory;
    double used_memory;
};

/**
 * @brief Parses /proc/meminfo lines into a mem struct.
 *
 * This version is more robust and doesn't modify the input lines.
 */
struct mem get_meminfo();

struct uptime {
    size_t seconds;
    size_t minutes;
    size_t hours;
    size_t days;
};

struct uptime get_uptime();

struct disk {
    double total_memory_gb;
    double used_memory_gb;
};

/**
 * @brief Gets disk info for the root filesystem "/".
 *
 * Corrected to use double for GiB calculations.
 */
struct disk get_diskinfo();

struct cpu {
    char *name;
    int cores;
};

struct cpu get_cpu();

char *get_hostname();
char *get_kernel();

#endif // SYSTEM_H
