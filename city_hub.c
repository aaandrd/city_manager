#include "city_manager.h"

// child processes

void monitor_child (int out_fd) {
    if (dup2(out_fd, 1) < 0) {
        printf("dup2 error\n");
        exit(4);
    }
    execl("./monitor_reports", "monitor_reports", NULL);
    printf("execl error\n");
    exit(5);
}

void scorer_child (char *district, int out_fd) {
    if (dup2(out_fd, 1) < 0) {
        printf("dup2 error\n");
        exit(4);
    }
    execl("./scorer", "scorer", district, NULL);
    printf("execl error\n");
    exit(5);
}

void hub_mon_process () {
    int p[2];
    int pid;
    char buf[128];
    int bytes;

    if (pipe(p) < 0) {
        printf("pipe error\n");
        exit(2);
    }

    if ((pid = fork()) < 0) {
        printf("fork error child\n");
        exit(3);
    }

    if (pid == 0) {
        close(p[0]); 
        monitor_child(p[1]);
    }

    // parent part of hub_mon
    close(p[1]); 
    printf("[Hub_Mon] Background monitor process started.\n");

    while ((bytes = read(p[0], buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = '\0';
        printf("\n[From Monitor] %s", buf);
    }

    printf("\n[Hub_Mon] Monitor process has ended.\n");
    close(p[0]);
    exit(0);
}

// signal handler

void display_child_status (int s) {
    int status, pid;
    
    if ((pid = wait(&status)) > 0) {
        if (WIFEXITED(status)) {
            // silently reap background processes
        }
    }
}

// main logic

int main (int argc, char *argv[]) {
    int pid;
    char input[256];
    struct sigaction sa;

    sa.sa_handler = display_child_status;
    // required so fgets doesn't crash on signal
    sa.sa_flags = SA_RESTART; 
    
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        printf("sigaction error\n");
        exit(4);
    }

    printf(C_LIST "Welcome to City Hub. Available commands:\n");
    printf(" - start_monitor\n");
    printf(" - calculate_scores <dist1> [dist2...]\n");
    printf(" - exit\n" RESET);

    while (1) {
        printf(C_ADD "\ncity_hub> " RESET);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0; // remove newline
        if (strlen(input) == 0) continue;

        char *cmd = strtok(input, " ");

        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            // clean up monitor before exiting
            int fd = open(".monitor_pid", 0); // 0 is O_RDONLY
            if (fd >= 0) {
                char pid_buf[32];
                int b = read(fd, pid_buf, 31);
                if (b > 0) {
                    pid_buf[b] = '\0';
                    kill(atoi(pid_buf), SIGINT);
                }
                close(fd);
            }
            
            printf("Exiting City Hub...\n");
            break;
        } 
        else if (strcmp(cmd, "start_monitor") == 0) {
            if ((pid = fork()) < 0) {
                printf("fork error hub_mon\n");
            } 
            if (pid == 0) {
                hub_mon_process();
            }
        } 
        else if (strcmp(cmd, "calculate_scores") == 0) {
            char *district;
            
            while ((district = strtok(NULL, " ")) != NULL) {
                int p[2];

                if (pipe(p) < 0) {
                    printf("pipe error\n");
                    continue;
                }

                if ((pid = fork()) < 0) {
                    printf("fork error scorer\n");
                    continue;
                }

                if (pid == 0) {
                    close(p[0]); // close read end
                    scorer_child(district, p[1]); 
                }

                close(p[1]); // close write end
                char buf[512];
                int bytes;

                printf("=== Workload Report: %s ===\n", district);
                while ((bytes = read(p[0], buf, sizeof(buf) - 1)) > 0) {
                    buf[bytes] = '\0';
                    printf("%s", buf);
                }
                close(p[0]);
            }
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}