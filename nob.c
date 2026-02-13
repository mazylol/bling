#include <stdlib.h>

#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define BINARY_NAME "bling"
#define INSTALL_PATH "/usr/local/bin/" BINARY_NAME

int command_exists(const char *cmd) {
    char shell_cmd[256];
    snprintf(shell_cmd, sizeof(shell_cmd), "command -v %s >/dev/null 2>&1", cmd);

    int status = system(shell_cmd);

    if (status == -1) {
        perror("system error");
        return 0;
    } else {
        int exit_status = WEXITSTATUS(status);

        if (exit_status == 0) {
            return 1;
        } else {
            return 0;
        }
    }
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    // Configuration
    const char *cc = "clang";
    const char *cflags[] = {"-Wall", "-Wextra", "-g", "-std=c99"}; // Add common flags here
    const char *libs[] = {};

    // Sources
    const char *sources[] = {
        SRC_FOLDER "main.c",
        SRC_FOLDER "file.c"};

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER))
        return 1;

    Nob_Cmd cmd = {0};
    Nob_File_Paths object_files = {0};
    Nob_Procs procs = {0};

    // Compile source files to objects
    for (size_t i = 0; i < NOB_ARRAY_LEN(sources); ++i) {
        const char *src_path = sources[i];

        const char *obj_path = nob_temp_sprintf("%s%s.o", BUILD_FOLDER, nob_path_name(src_path));

        // Add to the list of objects for the linker later
        nob_da_append(&object_files, obj_path);

        // Check if the source needs recompilation
        if (nob_needs_rebuild1(obj_path, src_path)) {
            cmd.count = 0;
            nob_cmd_append(&cmd, cc);
            nob_cmd_append(&cmd, "-c", src_path);
            nob_cmd_append(&cmd, "-o", obj_path);
            nob_da_append_many(&cmd, cflags, NOB_ARRAY_LEN(cflags));

            // Run async
            if (!nob_cmd_run(&cmd, .async = &procs))
                return 1;
        }
    }

    // Wait for comp to finish
    if (!nob_procs_wait(procs))
        return 1;

    const char *binary_path = BUILD_FOLDER BINARY_NAME;

    // Check if the binary needs relinking (if binary is missing OR any object file is newer)
    if (nob_needs_rebuild(binary_path, object_files.items, object_files.count)) {
        cmd.count = 0;

        // If bear exists, use it to generate compile_commands.json
        if (command_exists("bear")) {
            nob_cmd_append(&cmd, "bear", "--output", BUILD_FOLDER "compile_commands.json", "--");
        }

        nob_cmd_append(&cmd, cc);
        nob_cmd_append(&cmd, "-o", binary_path);
        // Append all object files
        nob_da_append_many(&cmd, object_files.items, object_files.count);
        // Append libraries (usually last)
        nob_da_append_many(&cmd, libs, NOB_ARRAY_LEN(libs));

        if (!nob_cmd_run(&cmd))
            return 1;
    }

    // Install Step
    if (argc > 1 && strcmp(argv[1], "install") == 0) {
        nob_log(NOB_INFO, "Installing %s to %s...", binary_path, INSTALL_PATH);

        if (!nob_copy_file(binary_path, INSTALL_PATH)) {
            nob_log(NOB_ERROR, "Installation failed. Do you need 'sudo'?");
            return 1;
        }
        nob_log(NOB_INFO, "Successfully installed!");
    }

    // Cleanup
    nob_cmd_free(cmd);
    nob_da_free(object_files);
    nob_da_free(procs);

    return 0;
}
