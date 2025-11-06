// SPDX-License-Identifier: GPL-3.0-or-later

#include <ctype.h> // For isspace()
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

#include "LICENSE.h"
#include "colors.h"
#include "file.h"

#define BUFFER_SIZE 256
const unsigned long long SEC_PER_MIN = 60;
const unsigned long long SEC_PER_HOUR = 3600;
const unsigned long long SEC_PER_DAY = 86400;

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
struct os parse_os(char **fileLines, int num_lines) {
    struct os os;
    os.name = NULL;
    os.version = NULL;
    os.build_id = NULL;

    char *name_ptr = NULL;
    char *version_ptr = NULL;
    char *build_id_ptr = NULL;

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

    return os;
}

struct mem {
    double max_memory;
    double used_memory;
};

/**
 * @brief Parses /proc/meminfo lines into a mem struct.
 *
 * This version is more robust and doesn't modify the input lines.
 */
struct mem parse_meminfo(char **fileLines, int num_lines) {
    struct mem mem = {0.0, 0.0};
    double total_mem_kb = 0.0;
    double avail_mem_kb = 0.0;

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

    return mem;
}

struct uptime {
    size_t seconds;
    size_t minutes;
    size_t hours;
    size_t days;
};

struct uptime parse_uptime(char *uptime_line) {
    struct uptime up = {0, 0, 0, 0};
    if (uptime_line == NULL) {
        return up;
    }

    // strtoull stops at the first non-numeric char (the space)
    unsigned long long total_seconds = strtoull(uptime_line, NULL, 10);

    up.days = total_seconds / SEC_PER_DAY;
    up.hours = (total_seconds % SEC_PER_DAY) / SEC_PER_HOUR;
    up.minutes = (total_seconds % SEC_PER_HOUR) / SEC_PER_MIN;
    up.seconds = total_seconds % SEC_PER_MIN;

    return up;
}

struct disk {
    double total_memory_gb;
    double used_memory_gb;
};

/**
 * @brief Gets disk info for the root filesystem "/".
 *
 * Corrected to use double for GiB calculations.
 */
struct disk get_diskinfo() {
    struct disk d = {0.0, 0.0};
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

// Main struct to hold system info
struct bling {
    char *username;
    char *hostname;
    struct os os;
    char *kernel;
    char *shell;

    struct mem mem;
    struct uptime uptime;
    struct disk disk;
};

/**
 * @brief Frees all heap-allocated memory within the bling struct.
 */
void free_bling(struct bling *b) {
    free(b->hostname);
    free(b->kernel);
    free(b->os.name);
    free(b->os.version);
    free(b->os.build_id);
    // b.username and b.shell point to getenv memory, no free needed
}

int main(int argc, char **argv) {
    const char *helpString = "bling, a very simple system info tool, kind of like neofetch but worse\n\n--help: this screen\n--license: view the license\n";

    if (argc != 1) {
        if (strcmp(argv[1], "--help") == 0) {
            printf("%s", helpString);
            return 0;
        } else if (strcmp(argv[1], "--license") == 0) {
            printf("%s", LICENSE);
            return 0;
        } else if (strstr(argv[1], "--") != NULL) {
            printf("%s", helpString);
            return 0;
        }
    }

    struct bling bling = {0};
    int num_lines = 0;
    char **fileLines = NULL;

    bling.username = getenv("USER");
    if (bling.username == NULL) {
        bling.username = "unknown";
    }

    fileLines = lines("/etc/hostname", &num_lines);
    if (fileLines != NULL && num_lines > 0) {
        bling.hostname = strdup(fileLines[0]);
    } else {
        bling.hostname = strdup("unknown");
    }
    free_string_array(fileLines);
    fileLines = NULL;

    fileLines = lines("/etc/os-release", &num_lines);
    bling.os = parse_os(fileLines, num_lines);
    free_string_array(fileLines);
    fileLines = NULL;

    fileLines = lines("/proc/version", &num_lines);
    if (fileLines != NULL && num_lines > 0) {
        char *kernel_string = split(fileLines[0], " ")[2];
        bling.kernel = strdup(kernel_string);
        free_string_array(fileLines);
        fileLines = NULL;
    } else {
        bling.kernel = strdup("unknown");
    }

    bling.shell = getenv("SHELL");
    if (bling.shell != NULL) {
        char *shell_name = strrchr(bling.shell, '/');
        if (shell_name != NULL) {
            bling.shell = shell_name + 1; // Point to just the name
        }
    } else {
        bling.shell = "unknown";
    }

    fileLines = lines("/proc/meminfo", &num_lines);
    bling.mem = parse_meminfo(fileLines, num_lines);
    free_string_array(fileLines);
    fileLines = NULL;

    bling.disk = get_diskinfo();

    fileLines = lines("/proc/uptime", &num_lines);
    if (fileLines != NULL && num_lines > 0) {
        bling.uptime = parse_uptime(fileLines[0]);
    }
    free_string_array(fileLines);
    fileLines = NULL;

    char user_host_buffer[BUFFER_SIZE];
    snprintf(user_host_buffer, BUFFER_SIZE, "%suser/host%s %s@%s", BHGRN, CRESET, bling.username, bling.hostname);

    char os_buffer[BUFFER_SIZE];
    if (bling.os.name != NULL && bling.os.version != NULL && bling.os.build_id != NULL) {
        snprintf(os_buffer, BUFFER_SIZE, "%sos%s        %s %s (%s)", BHCYN, CRESET, bling.os.name, bling.os.version, bling.os.build_id);
    } else if (bling.os.name != NULL && bling.os.version != NULL) {
        snprintf(os_buffer, BUFFER_SIZE, "%sos%s        %s %s", BHCYN, CRESET, bling.os.name, bling.os.version);
    } else if (bling.os.name != NULL) {
        snprintf(os_buffer, BUFFER_SIZE, "%sos%s        %s", BHCYN, CRESET, bling.os.name);
    } else {
        snprintf(os_buffer, BUFFER_SIZE, "%sos%s        unknown", BHCYN, CRESET);
    }

    printf("%s\n", user_host_buffer);
    printf("%s\n", os_buffer);
    printf("%skernel%s    %s\n", BHYEL, CRESET, bling.kernel);
    printf("%sshell%s     %s\n", BHMAG, CRESET, bling.shell);
    printf("%sram%s       %.1f / %.1f GiB\n", BHBLU, CRESET, bling.mem.used_memory, bling.mem.max_memory);
    printf("%suptime%s    %zud %zuh %zum %zus\n", BHBLK, CRESET, bling.uptime.days, bling.uptime.hours, bling.uptime.minutes, bling.uptime.seconds); // %zu for size_t
    printf("%sdisk%s      %.1f / %.1f GiB\n", BHRED, CRESET, bling.disk.used_memory_gb, bling.disk.total_memory_gb);

    free_bling(&bling);

    return 0;
}