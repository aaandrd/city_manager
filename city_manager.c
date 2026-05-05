#include "city_manager.h"

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
    int threshold_val = -1;

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
                    threshold_val = atoi(argv[++i]);
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
        manage_symlink(district);
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
    } else if (strcmp(command, "--remove_report") == 0) {
        if (report_id == -1) return 1;
        remove_report(district, report_id, role);
        log_operation(district, role, user, "remove_report");
    } else if (strcmp(command, "--update_threshold") == 0) {
        if (threshold_val == -1) return 1;
        update_threshold(district, threshold_val, role);
        log_operation(district, role, user, "update_threshold");
    } else if (strcmp(command, "--filter") == 0) {
        int start_idx = 0;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--filter") == 0) {
                start_idx = i + 2;
                break;
            }
        }
        int num_cond = argc - start_idx;
        filter_reports(district, num_cond, &argv[start_idx]);
        log_operation(district, role, user, "filter");
    } else if (strcmp(command, "--remove_district") == 0) {
        remove_district(district, role);
        log_operation(district, role, user, "remove_district");
    } else {
        printf("Unknown command: %s\n", command);
        usage();
    }

    return 0;
}