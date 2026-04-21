#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

typedef struct {
    int id;
    char inspector[32];
    float latitude;
    float longitude;
    char category[16];
    int severity;
    time_t timestamp;
    char description[128];
} Report;

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

void setup_district_dir(char *district_name) {
    struct stat info = {0};
    if (stat(district_name, &info) == -1) {
        if (mkdir(district_name, 0700) != 0) {
            perror("Failed to create district directory");
            exit(1);
        }
        if (chmod(district_name, 0750) != 0) {
            perror("Failed to chmod district directory");
            exit(1);
        }
        printf("Created directory for district '%s' with 750 permissions\n", district_name);
    }
}

void setup_config_file(const char *district_name) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/district.cfg", district_name);

    struct stat st;
    if (stat(filepath, &st) == -1) {
        int fd = open(filepath, O_CREAT | O_WRONLY, 0640);
        if (fd != -1) {
            chmod(filepath, 0640);
            const char *default_threshold = "2\n";
            write(fd, default_threshold, strlen(default_threshold));
            close(fd);
        }
    }
}

void log_operation(const char *district_name, const char *role, const char *user, const char *action) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/logged_district", district_name);

    if (!check_write_permission(filepath, role)) {
        printf("Access Denied: You cannot log operations to %s as an %s.\n", filepath, role);
        return;
    }

    int fd = open(filepath, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd != -1) {
        chmod(filepath, 0644);
        char log_entry[256];
        snprintf(log_entry, sizeof(log_entry), "%ld\n%s\n%s %s\n", time(NULL), user, role, action);
        write(fd, log_entry, strlen(log_entry));
        close(fd);
    }
}

// add a report
void add_report(const char *district_name, const char *inspector_name) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_name);

    int fd = open(filepath, O_CREAT | O_WRONLY | O_APPEND, 0664);
    if (fd == -1) {
        perror("Failed to open reports.dat");
        return;
    }
    chmod(filepath, 0664);

    Report new_report;
    new_report = (Report) {0};
    strncpy(new_report.inspector, inspector_name, sizeof(new_report.inspector) - 1);
    new_report.timestamp = time(NULL);

    printf("X: ");
    if (scanf("%f", &new_report.latitude) != 1) new_report.latitude = 0.0f;
    printf("Y: ");
    if (scanf("%f", &new_report.longitude) != 1) new_report.longitude = 0.0f;

    int c;
    while ((c = getchar()) != '\n' && c != EOF); // clear buffer

    printf("Category (road/lighting/flooding/other): ");
    if (fgets(new_report.category, sizeof(new_report.category), stdin)) {
        new_report.category[strcspn(new_report.category, "\n")] = 0;
    }

    printf("Severity level (1/2/3): ");
    if (scanf("%d", &new_report.severity) != 1) {
        new_report.severity = 1;
    }
    while ((c = getchar()) != '\n' && c != EOF); // clear buffer

    printf("Description: ");
    if (fgets(new_report.description, sizeof(new_report.description), stdin)) {
        new_report.description[strcspn(new_report.description, "\n")] = 0;
    }

    struct stat st;
    if (stat(filepath, &st) == 0) {
        new_report.id = st.st_size / sizeof(Report);
    } else {
        new_report.id = 0;
    }

    if (write(fd, &new_report, sizeof(Report)) == sizeof(Report)) {
        printf("Successfully saved report ID %d!\n", new_report.id);
    }
    close(fd);
}

void list_reports(const char *district_name) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_name);

    struct stat st;
    if (stat(filepath, &st) == -1) {
        printf("No reports found for district %s.\n", district_name);
        return;
    }

    char perms[10];
    mode_to_string(st.st_mode, perms);

    printf("--- File Info ---\n");
    printf("Permissions: %s\nSize: %ld bytes\nLast Modified: %s",
           perms, st.st_size, ctime(&st.st_mtime));
    printf("-----------------\n");

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) return;

    Report r;
    printf("%-5s | %-15s | %-10s | %-10s\n", "ID", "Inspector", "Category", "Severity");
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("%-5d | %-15s | %-10s | %-10d\n", r.id, r.inspector, r.category, r.severity);
    }
    close(fd);
}

void view_report(const char *district_name, int report_id) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_name);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        printf("Could not open reports file for viewing.\n");
        return;
    }

    // jump to record
    off_t offset = (off_t) (report_id * sizeof(Report));
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("Failed to seek to report");
        close(fd);
        return;
    }

    Report r;
    if (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("\n--- Report Details (ID: %d) ---\n", r.id);
        printf("Inspector: %s\n", r.inspector);
        printf("Location: X: %.2f, Y: %.2f\n", r.latitude, r.longitude);
        printf("Category: %s\n", r.category);
        printf("Severity: %d\n", r.severity);
        printf("Timestamp: %s", ctime(&r.timestamp));
        printf("Description: %s\n", r.description);
        printf("-------------------------------\n");
    } else {
        printf("Report ID %d not found.\n", report_id);
    }
    close(fd);
}

// print usage
void usage() {
    printf("Usage: city_manager --role <inspector|manager> --user <name> [operation] [args...]\n");
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        usage();
        return 1;
    }

    char *role = NULL;
    char *user = NULL;
    char *command = NULL;
    char *district = NULL;
    int report_id = -1;

    // parse args
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        } else if (command == NULL) {
            command = argv[i];
            if (i + 1 < argc) {
                district = argv[++i];
                if (i + 1 < argc && argv[i+1][0] != '-') {
                    report_id = atoi(argv[++i]);
                }
            }
        }
    }

    // check if we got what we need
    if (!role || !user || !command || !district) {
        printf("Error: Missing required arguments.\n");
        usage();
        return 1;
    }

    // do the commands
    if (strcmp(command, "--add") == 0) {
        setup_district_dir(district);
        setup_config_file(district);
        add_report(district, user);
        log_operation(district, role, user, "add");
    }
    else if (strcmp(command, "--list") == 0) {
        list_reports(district);
        log_operation(district, role, user, "list");
    }
    else if (strcmp(command, "--view") == 0) {
        if (report_id == -1) {
            printf("Error: --view requires a report ID.\n");
            return 1;
        }
        view_report(district, report_id);
        log_operation(district, role, user, "view");
    }
    else {
        printf("Unknown command: %s\n", command);
        usage();
    }

    return 0;
}