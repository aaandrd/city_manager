#include "city_manager.h"


void mode_to_string(mode_t mode, char *str) {
    strcpy(str, "---------");
    if (mode & S_IRUSR) str[0] = 'r';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IXUSR) str[2] = 'x';
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IXGRP) str[5] = 'x';
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IXOTH) str[8] = 'x';
}

int check_write_permission(const char *filepath, const char *role) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        return 1; // let it create
    }

    if (strcmp(role, "manager") == 0) {
        if (!(st.st_mode & S_IWUSR)) {
            printf("Error: Manager does not have owner-write permission for %s.\n", filepath);
            return 0;
        }
    } else if (strcmp(role, "inspector") == 0) {
        if (!(st.st_mode & S_IWGRP)) {
            printf("Error: Inspector does not have group-write permission for %s.\n", filepath);
            return 0;
        }
    } else {
        printf("Error: Unknown role.\n");
        return 0;
    }
    return 1;
}
