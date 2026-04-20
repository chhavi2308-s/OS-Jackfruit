#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#define FILE_NAME "containers.txt"

// ---------------- START CONTAINER ----------------
void start_container(char *id, char *rootfs, char *cmd) {
    pid_t pid = fork();

    if (pid == 0) {
        // CHILD PROCESS

        if (chroot(rootfs) != 0) {
            perror("chroot failed");
            exit(1);
        }

        chdir("/");

        // 🔥 LOGGING (redirect stdout & stderr)
        int fd = open("/container.log", O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (fd < 0) {
            perror("log file open failed");
            exit(1);
        }

        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);

        char *args[] = {cmd, NULL};
        execvp(cmd, args);

        perror("exec failed");
        exit(1);
    } 
    else {
        // PARENT PROCESS

        FILE *f = fopen(FILE_NAME, "a");
        if (f != NULL) {
            fprintf(f, "%s %d running\n", id, pid);
            fclose(f);
        }

        printf("Container %s started (PID %d)\n", id, pid);
    }
}

// ---------------- LIST CONTAINERS ----------------
void list_containers() {
    FILE *f = fopen(FILE_NAME, "r");

    if (!f) {
        printf("No containers found\n");
        return;
    }

    char id[50], state[50];
    int pid;

    printf("ID\tPID\tSTATE\n");

    while (fscanf(f, "%s %d %s", id, &pid, state) != EOF) {
        printf("%s\t%d\t%s\n", id, pid, state);
    }

    fclose(f);
}

// ---------------- STOP CONTAINER ----------------
void stop_container(char *id) {
    FILE *f = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.txt", "w");

    if (!f || !temp) {
        printf("Error opening file\n");
        return;
    }

    char cid[50], state[50];
    int pid;

    while (fscanf(f, "%s %d %s", cid, &pid, state) != EOF) {
        if (strcmp(cid, id) == 0) {
            kill(pid, SIGKILL);
            fprintf(temp, "%s %d stopped\n", cid, pid);
            printf("Container %s stopped\n", cid);
        } else {
            fprintf(temp, "%s %d %s\n", cid, pid, state);
        }
    }

    fclose(f);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);
}

// ---------------- MAIN ----------------
int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage:\n");
        printf("./engine start <id> <rootfs> <cmd>\n");
        printf("./engine ps\n");
        printf("./engine stop <id>\n");
        return 1;
    }

    if (strcmp(argv[1], "start") == 0) {
        if (argc < 5) {
            printf("Usage: start <id> <rootfs> <cmd>\n");
            return 1;
        }
        start_container(argv[2], argv[3], argv[4]);
    }

    else if (strcmp(argv[1], "ps") == 0) {
        list_containers();
    }

    else if (strcmp(argv[1], "stop") == 0) {
        if (argc < 3) {
            printf("Usage: stop <id>\n");
            return 1;
        }
        stop_container(argv[2]);
    }

    else {
        printf("Invalid command\n");
    }

    return 0;
}
