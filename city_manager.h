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
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>

#define DEFAULT_THRESHOLD "2" //default severity threshold

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
void manage_symlink(const char *district_name);
void remove_district(const char *district_name, const char *role);
int notify_monitor();

//ANSI COLOR MACROS
#define RESET       "\033[0m"
#define BOLD        "\033[1m"

//custom RGB colors
#define C_ADD              "\033[38;2;143;255;227m" // Cyan/Green
#define C_LIST             "\033[38;2;220;152;255m" // Purple
#define C_VIEW             "\033[38;2;255;154;172m" // Pink
#define C_FILTER           "\033[38;2;250;237;120m" // Yellow


#define C_ERROR     "\033[31m" // Standard Red
#define C_SUCCESS   "\033[32m" // Standard Green
#define C_WARN      "\033[33m" // Standard Yellow
#define C_INFO      "\033[36m" // Standard Cyan

#endif //CITY_MANAGER_CITY_MANAGER_H