// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

// No filesystem, console transport or heap is provided by this platform. Report
// failure explicitly if libc attempts to use one; never discard output while
// reporting a successful write. The set is complete so that the linker never
// pulls a toolchain's own syscalls object (GCC 15 newlib pulls it for _sbrk).
int _close(int fd) {
    (void)fd;
    errno = EBADF;
    return -1;
}

off_t _lseek(int fd, off_t offset, int whence) {
    (void)fd;
    (void)offset;
    (void)whence;
    errno = ESPIPE;
    return (off_t)-1;
}

int _read(int fd, void *buffer, size_t count) {
    (void)fd;
    (void)buffer;
    (void)count;
    errno = ENOSYS;
    return -1;
}

int _write(int fd, const void *buffer, size_t count) {
    (void)fd;
    (void)buffer;
    (void)count;
    errno = ENOSYS;
    return -1;
}

int _fstat(int fd, struct stat *status) {
    (void)fd;
    (void)status;
    errno = EBADF;
    return -1;
}

int _isatty(int fd) {
    (void)fd;
    errno = ENOTTY;
    return 0;
}

void *_sbrk(ptrdiff_t increment) {
    (void)increment;
    errno = ENOMEM;
    return (void *)-1;
}

int _getpid(void) {
    return 1;
}

int _kill(int pid, int signal) {
    (void)pid;
    (void)signal;
    errno = EINVAL;
    return -1;
}

__attribute__((noreturn)) void _exit(int status) {
    (void)status;
    for (;;) {
    }
}
