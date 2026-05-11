#ifndef MONITOR_REPORTS_H
#define MONITOR_REPORTS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

void set_handler(int signum, void (*handler)(int));
void monitor_ends(int sig);
void monitor_writes(int sig);

#endif /* MONITOR_REPORTS_H */