#include "monitor_reports.h"
#include "city_manager.h"

int counter = 0; 

void monitor_ends (int sig) {
    printf(C_FILTER "INFO: Monitor received SIGINT. Shutting down.\n" RESET);
    fflush(stdout);
    unlink(".monitor_pid");
    exit(0);
}

void monitor_writes (int sig) {
    counter++;
    printf(C_LIST "EVENT: Monitor received SIGUSR1 (Count: %d)\n" RESET, counter);
    fflush(stdout);
}

int main () {
    int fd_check;
    int fd;
    int bytes;
    char buf[32];
    char pid_str[32];
    struct sigaction sa_int, sa_usr;

    // check if another monitor is already running (0 is O_RDONLY)
    fd_check = open(".monitor_pid", 0); 
    if (fd_check >= 0) {
        bytes = read(fd_check, buf, 31);
        if (bytes > 0) {
            buf[bytes] = '\0';
            printf(C_ERROR "ERROR: Another monitor is already running with PID: %d\n" __REGISTER_PREFIX__, atoi(buf));
        }
        close(fd_check);
        exit(1); 
    }

    // create pid file
    fd = open(".monitor_pid", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        printf(C_ERROR "ERROR: Could not create .monitor_pid\n" RESET);
        exit(2);
    }

    sprintf(pid_str, "%d\n", getpid());
    write(fd, pid_str, strlen(pid_str));
    close(fd);

    printf(C_FILTER "INFO: Monitor started with PID: %d\n" RESET , getpid());
    fflush(stdout); 

    // setup sigint handler
    sa_int.sa_handler = monitor_ends;
    sa_int.sa_flags = 0;
    sigemptyset(&sa_int.sa_mask);
    if (sigaction(SIGINT, &sa_int, NULL) < 0) {
        printf("sigaction error\n");
        exit(3);
    }

    // setup sigusr1 handler
    sa_usr.sa_handler = monitor_writes;
    sa_usr.sa_flags = 0;
    sigemptyset(&sa_usr.sa_mask);
    if (sigaction(SIGUSR1, &sa_usr, NULL) < 0) {
        printf("sigaction error\n");
        exit(4);
    }

    while (1) {
        pause();
    }

    return 0;
}