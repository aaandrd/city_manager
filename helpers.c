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

int parse_condition(const char *input, char *field, char *op, char *value) {
    // %[^:] means "read a string until you hit a colon"
    // We expect exactly 3 items to be successfully read
    if (sscanf(input, "%[^:]:%[^:]:%s", field, op, value) == 3) {
        return 1;
    }
    return 0;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    // 1. Numeric Comparison: Severity
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);
        if (strcmp(op, "==") == 0) return r->severity == val;
        if (strcmp(op, "!=") == 0) return r->severity != val;
        if (strcmp(op, ">")  == 0) return r->severity > val;
        if (strcmp(op, "<")  == 0) return r->severity < val;
        if (strcmp(op, ">=") == 0) return r->severity >= val;
        if (strcmp(op, "<=") == 0) return r->severity <= val;
    }
    // 2. Numeric Comparison: Timestamp
    else if (strcmp(field, "timestamp") == 0) {
        long val = atol(value); // time_t is generally a long
        if (strcmp(op, "==") == 0) return r->timestamp == val;
        if (strcmp(op, "!=") == 0) return r->timestamp != val;
        if (strcmp(op, ">")  == 0) return r->timestamp > val;
        if (strcmp(op, "<")  == 0) return r->timestamp < val;
        if (strcmp(op, ">=") == 0) return r->timestamp >= val;
        if (strcmp(op, "<=") == 0) return r->timestamp <= val;
    }
    // 3. String Comparison: Category
    else if (strcmp(field, "category") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->category, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->category, value) != 0;
    }
    // 4. String Comparison: Inspector
    else if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->inspector, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->inspector, value) != 0;
    }

    return 0; // Return 0 if the field or operator is invalid
}