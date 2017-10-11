/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
 
void _exit(); // implemented in libtransistor crt0

int _close(int file) {
  return -1;
}

char *_environ[] = {NULL};

int _execve(char *name, char **argv, char **env) {
  return -1;
}

int _fork() {
  return -1;
}

int _fstat(int file, struct stat *st) {
  return -1;
}

int _getpid() {
  return -1;
}

int _isatty(int file) {
  return -1;
}

int _kill(int pid, int sig) {
  return -1;
}

int _link(char *old, char *new) {
  return -1;
}

int _lseek(int file, int ptr, int dir) {
  return -1;
}

int _open(const char *name, int flags, ...) {
  return -1;
}

int _read(int file, char *ptr, int len) {
  return -1;
}

caddr_t _sbrk(int incr) {
  return (void*) -1;
}

int _stat(const char *file, struct stat *st) {
  return -1;
}

clock_t _times(struct tms *buf) {
  return (clock_t) -1;
}

int _unlink(char *name) {
  return -1;
}

int _wait(int *status) {
  return -1;
}

int _write(int file, char *ptr, int len) {
  return -1;
}

int _gettimeofday(struct timeval *__restrict p, void *__restrict z) {
  return -1;
}
