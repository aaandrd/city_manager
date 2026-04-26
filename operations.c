#include "city_manager.h"

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
            write(fd, DEFAULT_THRESHOLD, strlen(DEFAULT_THRESHOLD));
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
        long int timestamp = time(NULL);
        snprintf(log_entry, sizeof(log_entry), "%s%s\n%s %s\n", ctime(&timestamp), user, role, action);
        write(fd, log_entry, strlen(log_entry));
        close(fd);
    }
}

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

    printf("=== File Info ===\n");
    printf("Permissions: %s\nSize: %ld bytes\nLast Modified: %s",
           perms, st.st_size, ctime(&st.st_mtime));
    printf("=================\n");

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

// remove one report and shift the rest
void remove_report(const char *district_name, int report_id, const char *role) {
    if (strcmp(role, "manager") != 0) {
        printf("managers only.\n");
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_name);

    int fd = open(filepath, O_RDWR);
    if (fd == -1) return;

    struct stat st;
    fstat(fd, &st);
    int total = st.st_size / sizeof(Report);

    if (report_id >= total || report_id < 0) {
        printf("Bad id.\n");
        close(fd);
        return;
    }
    Report temp;
    // shift everything over by one
    for (int i = report_id + 1; i < total; i++) {
        lseek(fd, i * sizeof(Report), SEEK_SET);
        read(fd, &temp, sizeof(Report));
        temp.id = i - 1; // fix the id
        lseek(fd, (i - 1) * sizeof(Report), SEEK_SET);
        write(fd, &temp, sizeof(Report));
    }

    // rremove the duplicate end
    ftruncate(fd, (total - 1) * sizeof(Report));
    close(fd);
    printf("Report removed\n");
}

void update_threshold(const char *district_name, int value, const char *role) {
    if (strcmp(role, "manager") != 0) {
        printf("managers only.\n");
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/district.cfg", district_name);

    struct stat st;
    if (stat(filepath, &st) == -1) return;

    if ((st.st_mode & 0777) != 0640) {
        printf("Permissions changed,refusing to update.\n");
        return;
    }

    int fd = open(filepath, O_WRONLY | O_TRUNC);
    if (fd != -1) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d\n", value);
        write(fd, buf, strlen(buf));
        close(fd);
        printf("Threshold updated to %d\n", value);
    }
}

void filter_reports(const char *district_name, int condition_count, char **conditions) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_name);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) return;

    Report r;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int ok = 1;
        for (int i = 0; i < condition_count; i++) {
            char f[32], o[4], v[32];
            if (parse_condition(conditions[i], f, o, v)) {
                if (!match_condition(&r, f, o, v)) ok = 0;
            }
        }
        if (ok) printf("ID: %d | Cat: %s | Sev: %d\n", r.id, r.category, r.severity);
    }
    close(fd);
}