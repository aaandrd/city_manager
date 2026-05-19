#include "city_manager.h"

typedef struct {
    char name[32];
    int total_severity;
} InspectorScore;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <district_name>\n", argv[0]);
        return 1;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", argv[1]);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        printf("No reports found or cannot access district: %s\n", argv[1]);
        return 0;
    }

    InspectorScore scores[100];
    int score_count = 0;
    Report r;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int found = 0;
        for (int i = 0; i < score_count; i++) {
            if (strcmp(scores[i].name, r.inspector) == 0) {
                scores[i].total_severity += r.severity;
                found = 1;
                break;
            }
        }
        if (!found && score_count < 100) {
            strncpy(scores[score_count].name, r.inspector, 31);
            scores[score_count].total_severity = r.severity;
            score_count++;
        }
    }
    close(fd);

    //summary  will be piped back to city_hub
    if (score_count == 0) {
        printf("No active inspectors in %s.\n", argv[1]);
    } else {
        for (int i = 0; i < score_count; i++) {
            printf("- %s: Workload Score = %d\n", scores[i].name, scores[i].total_severity);
        }
    }

    return 0;
}