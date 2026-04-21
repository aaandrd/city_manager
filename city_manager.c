#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

// TO DO
    // add <district_id>
    // list <district_id>
    // view <district_id> <report_id>
    // remove_report <district_id> <report_id>
    // update_threshold <district_id> <value>
    // filter <district_id> <condition>

typedef struct {
    int id;
    char inspector[32];
    float latitude;
    float longitude;
    char category[16];
    int severity;   // 1 = minor, 2 = moderate, 3 = critical
    time_t timestamp;
    char description[128];
} Report;

void setup_district_dir(char *district_name) {
    struct stat info = {0};
    if (stat(district_name, &info) == -1) {
        //dir doesnt exist yet, create with basic permissions
        if (mkdir(district_name, 0700) != 0) {
            perror("Failed to create district directory");
            exit(1);
        }

        //set permissions to rwxr-x-- (0750)
        if (chmod(district_name, 0750) != 0) {
            perror("Failed to chmod district directory");
            exit(1);
        }

        printf ("Created directory fo the district '%s' with 7500 permissions\n", district_name);
    } else {
        printf ("Directory '%s' already exists\n", district_name);
    }
}

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

    //argument parsing
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        } else if (command == NULL) {
            command = argv[i];
            if (i + 1 < argc) {
                district = argv[++i];
            }
        }
    }

    if (!role || !user || !command || !district) {
        printf("Error: Missing required arguments.\n");
        usage();
        return 1;
    }

    if (strcmp(command, "--add") == 0) {
        printf("Role: %s, User: %s\n", role, user);
        printf("Executing ADD for district: %s\n", district);
        setup_district_dir(district);
    }
    else if (strcmp(command, "--list") == 0) {
        printf("Executing LIST for district: %s\n", district);
    }

    else {
        printf("Unknown command: %s\n", command);
        usage();
    }

    return 0;
}