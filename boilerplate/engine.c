#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>

#include "monitor_ioctl.h"

#define STACK_SIZE (1024 * 1024)
#define CONTAINER_ID_LEN 32
#define CONTROL_PATH "/tmp/mini_runtime.sock"

typedef enum { CMD_SUPERVISOR = 0, CMD_START, CMD_PS, CMD_LOGS, CMD_STOP } command_kind_t;

typedef struct container_record {
    char id[CONTAINER_ID_LEN];
    pid_t host_pid;
    char state[20];
    int log_read_fd;
    struct container_record *next;
} container_record_t;

typedef struct {
    command_kind_t kind;
    char container_id[CONTAINER_ID_LEN];
    char rootfs[PATH_MAX];
    char command[256];
    int pipe_write_fd;
    int nice_val;
} control_request_t;

container_record_t *container_list = NULL;
pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

static int container_main(void *arg) {
    control_request_t *req = (control_request_t *)arg;
    
    setpriority(PRIO_PROCESS, 0, req->nice_val);
    dup2(req->pipe_write_fd, STDOUT_FILENO);
    dup2(req->pipe_write_fd, STDERR_FILENO);
    close(req->pipe_write_fd);

    if (chroot(req->rootfs) != 0) return -1;
    if (chdir("/") != 0) return -1;
    mount("proc", "/proc", "proc", 0, NULL);

    char *args[] = {req->command, "30", NULL};
    execv(req->command, args);
    return 0;
}

void handle_client(int client_fd) {
    control_request_t req;
    if (read(client_fd, &req, sizeof(req)) <= 0) return;

    if (req.kind == CMD_START) {
        int pipefds[2];
        if (pipe(pipefds) < 0) return;
        req.pipe_write_fd = pipefds[1];

        char *stack = malloc(STACK_SIZE);
        pid_t child_pid = clone(container_main, stack + STACK_SIZE, 
                               CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | SIGCHLD, &req);
        
        close(pipefds[1]); 
        fcntl(pipefds[0], F_SETFL, O_NONBLOCK);

        pthread_mutex_lock(&list_lock);
        container_record_t *new_c = malloc(sizeof(container_record_t));
        strncpy(new_c->id, req.container_id, CONTAINER_ID_LEN);
        new_c->host_pid = child_pid;
        new_c->log_read_fd = pipefds[0];
        strcpy(new_c->state, "RUNNING");
        new_c->next = container_list;
        container_list = new_c;
        pthread_mutex_unlock(&list_lock);
        
        printf("[Supervisor] Started %s (PID: %d) with nice %d\n", req.container_id, child_pid, req.nice_val);
    } 
    else if (req.kind == CMD_PS) {
        pthread_mutex_lock(&list_lock);
        container_record_t *curr = container_list;
        while (curr) {
            write(client_fd, curr, sizeof(container_record_t));
            curr = curr->next;
        }
        pthread_mutex_unlock(&list_lock);
    }
    else if (req.kind == CMD_LOGS) {
        pthread_mutex_lock(&list_lock);
        container_record_t *curr = container_list;
        while(curr) {
            if(strcmp(curr->id, req.container_id) == 0) {
                char buf[4096];
                int n = read(curr->log_read_fd, buf, sizeof(buf)-1);
                if(n > 0) write(client_fd, buf, n);
                else write(client_fd, "No new logs.\n", 13);
                break;
            }
            curr = curr->next;
        }
        pthread_mutex_unlock(&list_lock);
    }
    close(client_fd);
}

int run_supervisor() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    unlink(CONTROL_PATH);
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, CONTROL_PATH, sizeof(addr.sun_path)-1);
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);
    printf("== SUPERVISOR STARTING ==\nListening on %s\n", CONTROL_PATH);
    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd >= 0) handle_client(client_fd);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    if (strcmp(argv[1], "supervisor") == 0) return run_supervisor();

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, CONTROL_PATH, sizeof(addr.sun_path)-1);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return 1;

    control_request_t req;
    memset(&req, 0, sizeof(req));
    if (strcmp(argv[1], "start") == 0 && argc >= 5) {
        req.kind = CMD_START;
        strncpy(req.container_id, argv[2], CONTAINER_ID_LEN);
        strncpy(req.rootfs, argv[3], PATH_MAX);
        strncpy(req.command, argv[4], 255);
        req.nice_val = (argc == 7 && strcmp(argv[5], "--nice") == 0) ? atoi(argv[6]) : 0;
        write(fd, &req, sizeof(req));
        printf("Request sent: %s\n", argv[2]);
    } 
    else if (strcmp(argv[1], "ps") == 0) {
        req.kind = CMD_PS;
        write(fd, &req, sizeof(req));
        container_record_t res;
        printf("ID\t\tPID\tSTATUS\n");
        while (read(fd, &res, sizeof(res)) > 0) 
            printf("%s\t%d\t%s\n", res.id, res.host_pid, res.state);
    }
    else if (strcmp(argv[1], "logs") == 0) {
        req.kind = CMD_LOGS;
        strncpy(req.container_id, argv[2], CONTAINER_ID_LEN);
        write(fd, &req, sizeof(req));
        char buf[4096];
        int n = read(fd, buf, sizeof(buf));
        if (n > 0) write(1, buf, n);
    }
    close(fd);
    return 0;
}
