#include "monitor_reports.h"

int n = 0;

void set_handler(int signum, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(signum, &sa, NULL) < 0) {
        printf("Error setting handler for signal %d\n", signum);
        exit(1);
    }
}

//handler for SIGINT, cleans up and exits directly
void monitor_ends(int sig) {
    (void)sig;
    printf("\nMonitor received SIGINT. Shutting down.\n");

    if (unlink(".monitor_pid") < 0) {
        printf("Error deleting .monitor_pid\n");
    } else {
        printf(".monitor_pid deleted.\n");
    }

    printf("Monitor ends.\n");
    exit(0);
}

//handler for SIGUSR1
void monitor_writes(int sig) {
    (void)sig;
    printf("Monitor received SIGUSR1: %d\n", ++n);
}

int main() {
    int fd = open(".monitor_pid", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        printf("Error creating .monitor_pid\n");
        exit(2);
    }

    char pid_str[32];
    sprintf(pid_str, "%d\n", getpid());
    if (write(fd, pid_str, strlen(pid_str)) < 0) {
        printf("Error writing to .monitor_pid\n");
        exit(3);
    }
    close(fd);

    printf("Monitor started with PID: %d\n", getpid());

    set_handler(SIGINT, monitor_ends);
    set_handler(SIGUSR1, monitor_writes);

    while(1) {
        pause();
    }

    return 0;
}