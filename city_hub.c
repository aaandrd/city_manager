#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    char input[256];
    
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    printf("Welcome to City Hub. Available commands:\n");
    printf(" - start_monitor\n");
    printf(" - calculate_scores <dist1> [dist2...]\n");
    printf(" - exit\n");

    while (1) {
        printf("\ncity_hub> ");
        if (!fgets(input, sizeof(input), stdin)) break;

        input[strcspn(input, "\n")] = 0; //remove nl
        if (strlen(input) == 0) continue;

        char *cmd = strtok(input, " ");

        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            printf("Exiting City Hub...\n");
            break;
        } 
        else if (strcmp(cmd, "start_monitor") == 0) {
            pid_t hub_mon_pid = fork();
            if (hub_mon_pid < 0) {
                perror("Fork failed for hub_mon");
            } else if (hub_mon_pid == 0) {
                //in hub_mon child
                int p[2];
                if (pipe(p) < 0) {
                    perror("Pipe failed");
                    exit(1);
                }

                pid_t mon_pid = fork();
                if (mon_pid < 0) {
                    perror("Fork failed for monitor");
                    exit(1);
                } else if (mon_pid == 0) {
                    //in the monitor process
                    close(p[0]); 
                    dup2(p[1], STDOUT_FILENO);
                    
                    execl("./monitor_reports", "monitor_reports", (char *)NULL);
                    perror("Failed to execute monitor_reports");
                    exit(1);
                } else {
                    //back in hub_mon
                    close(p[1]);
                    char buf[128];
                    ssize_t bytes;
                    
                    printf("[Hub_Mon] Background monitor process started.\n");
                    
                    //read from pipe
                    while ((bytes = read(p[0], buf, sizeof(buf) - 1)) > 0) {
                        buf[bytes] = '\0';
                        printf("\n[From Monitor] %s", buf);
                    }
                    
                    printf("\n[Hub_Mon] Monitor process has ended.\n");
                    close(p[0]);
                    exit(0); //hub_mon finishes when monitor dies
                }
            }
            //city_hub proc continues immediately, 
            //leaving hub_mon in the background
        } 
        else if (strcmp(cmd, "calculate_scores") == 0) {
            char *district;
            //go through all available distr
            while ((district = strtok(NULL, " ")) != NULL) {
                int p[2];
                if (pipe(p) < 0) {
                    perror("Pipe failed");
                    continue;
                }

                pid_t scorer_pid = fork();
                if (scorer_pid < 0) {
                    perror("Fork failed for scorer");
                } else if (scorer_pid == 0) {
                    //child process scorer
                    close(p[0]); //close read end
                    dup2(p[1], STDOUT_FILENO); //redirect stdout to pipe
                    close(p[1]); //close original write end

                    execl("./scorer", "scorer", district, (char *)NULL);
                    perror("Failed to execute scorer");
                    exit(1);
                } else {
                    //parent process city_hub reads the output
                    close(p[1]); //close write end
                    char buf[512];
                    ssize_t bytes;

                    printf("=== Workload Report: %s ===\n", district);
                    while ((bytes = read(p[0], buf, sizeof(buf) - 1)) > 0) {
                        buf[bytes] = '\0';
                        printf("%s", buf);
                    }
                    close(p[0]);

                    //wait for scorer to finish to prevent overlap
                    waitpid(scorer_pid, NULL, 0);
                }
            }
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}