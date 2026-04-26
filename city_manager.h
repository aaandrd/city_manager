#ifndef CITY_MANAGER_CITY_MANAGER_H
#define CITY_MANAGER_CITY_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define DEFAULT_THRESHOLD "2"

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

void mode_to_string(mode_t mode, char *str);
int check_write_permission(const char *filepath, const char *role);
void setup_district_dir(char *district_name);
void setup_config_file(const char *district_name);
void log_operation(const char *district_name, const char *role, const char *user, const char *action);
void add_report(const char *district_name, const char *inspector_name);
void list_reports(const char *district_name);
void view_report(const char *district_name, int report_id);
void remove_report(const char *district_name, int report_id, const char *role);
void update_threshold(const char *district_name, int value, const char *role);
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);
void filter_reports(const char *district_name, int condition_count, char **conditions);

#endif //CITY_MANAGER_CITY_MANAGER_H