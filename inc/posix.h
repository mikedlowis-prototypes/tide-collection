#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE   700
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static long writebuf(int fd, const void *buf, size_t nbyte) {
    long nwrite, total = 0;
    while (nbyte && ((nwrite = write(fd, buf, nbyte)) > 0))
        buf += nwrite, nbyte -= nwrite, total += nwrite;
    return (nwrite < 0 ? nwrite : total);
}
