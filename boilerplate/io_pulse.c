include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_OUTPUT "/tmp/io_pulse.out"

static unsigned int parse_uint(const char *arg, unsigned int fallback)
{
    char *end = NULL;
    unsigned long value = strtoul(arg, &end, 10);

    if (!arg || *arg == '\0' || (end && *end != '\0') || value == 0)
        return fallback;
    return (unsigned int)value;
}

int main(int argc, char *argv[])
{
    const unsigned int iterations = (argc > 1) ? parse_uint(argv[1], 20) : 20;
    const unsigned int sleep_ms = (argc > 2) ? parse_uint(argv[2], 200) : 200;
    int fd;
    unsigned int i;

    fd = open(DEFAULT_OUTPUT, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    for (i = 0; i < iterations; i++) {
        const char *msg = "io_pulse activity\n";
        if (write(fd, msg, strlen(msg)) < 0) perror("write");
        fsync(fd);
        printf("io_pulse: burst %u/%u\n", i + 1, iterations);
        usleep(sleep_ms * 1000);
    }

    close(fd);
    return 0;
}
