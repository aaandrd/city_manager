#include "monitor_reports.h"
int n = 0;

void set_handler(int signum, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(signum, &sa, NULL);
}

void monitor_ends(int sig) {
    (void)sig;
    printf("INFO: Monitor received SIGINT. Shutting down.\n");
    unlink(".monitor_pid");
    exit(0);
}

void monitor_writes(int sig) {
    (void)sig;
    printf("EVENT: Monitor received SIGUSR1 (Count: %d)\n", ++n);
}

int main() {
    //disable stdout buffering 
    setvbuf(stdout, NULL, _IONBF, 0);

    //check if another monitor is already running
    int fd_check = open(".monitor_pid", O_RDONLY);
    if (fd_check >= 0) {
        char buf[32];
        ssize_t bytes = read(fd_check, buf, sizeof(buf) - 1);
        if (bytes > 0) {
            buf[bytes] = '\0';
            printf("ERROR: Another monitor is already running with PID: %d\n", atoi(buf));
        }
        close(fd_check);
        exit(1); 
    }

    //make pid file
    int fd = open(".monitor_pid", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        printf("ERROR: Could not create .monitor_pid\n");
        exit(2);
    }

    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    write(fd, pid_str, strlen(pid_str));
    close(fd);

    printf("INFO: Monitor started with PID: %d\n", getpid());

    set_handler(SIGINT, monitor_ends);
    set_handler(SIGUSR1, monitor_writes);

    while(1) {
        pause();
    }
    return 0;
}