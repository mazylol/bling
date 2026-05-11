// SPDX-License-Identifier: GPL-3.0-or-later

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LICENSE.h"
#include "colors.h"
#include "system.h"

#define BUFFER_SIZE 256

// Main struct to hold system info
struct bling {
    char *username;
    char *hostname;
    struct os os;
    char *kernel;
    char *shell;

    struct cpu cpu;
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
    free(b->cpu.name);
    // b.username and b.shell point to getenv memory, no free needed
}

int main(int argc, char **argv) {
    const char *helpString = "bling, a very simple system info tool"
                             "\n\n--help: this screen\n--license: view the license\n";

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

    struct bling bling = { 0 };

    bling.username = getenv("USER");
    if (bling.username == NULL) {
        bling.username = "unknown";
    }

    bling.hostname = get_hostname();
    bling.os = get_os();
    bling.kernel = get_kernel();
    bling.mem = get_meminfo();
    bling.disk = get_diskinfo();
    bling.cpu = get_cpu();
    bling.uptime = get_uptime();

    bling.shell = getenv("SHELL");
    if (bling.shell != NULL) {
        char *shell_name = strrchr(bling.shell, '/');
        if (shell_name != NULL) {
            bling.shell = shell_name + 1; // Point to just the name
        }
    } else {
        bling.shell = "unknown";
    }

    char user_host_buffer[BUFFER_SIZE];
    snprintf(user_host_buffer, BUFFER_SIZE, "%suser/host%s %s@%s", BHGRN, CRESET, bling.username, bling.hostname);

    char os_buffer[BUFFER_SIZE];
    if (bling.os.name != NULL && bling.os.version != NULL && bling.os.build_id != NULL) {
        snprintf(os_buffer, BUFFER_SIZE, "%sos%s        %s %s (%s)", BHCYN, CRESET, bling.os.name, bling.os.version,
                 bling.os.build_id);
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
    printf("%scpu%s       %s (%d)\n", BHWHT, CRESET, bling.cpu.name, bling.cpu.cores);
    printf("%sram%s       %.1f / %.1f GiB\n", BHBLU, CRESET, bling.mem.used_memory, bling.mem.max_memory);
    printf("%suptime%s    %zud %zuh %zum %zus\n", BHBLK, CRESET, bling.uptime.days, bling.uptime.hours, bling.uptime.minutes,
           bling.uptime.seconds); // %zu for size_t
    printf("%sdisk%s      %.1f / %.1f GiB\n", BHRED, CRESET, bling.disk.used_memory_gb, bling.disk.total_memory_gb);

    free_bling(&bling);

    return 0;
}
