#include <errno.h>
#include <stdlib.h>

#include "fd.h"

#define FD_MAX 1024
static struct file *fds[FD_MAX] = {0};

int fd_create_file(struct file_operations *fops, void *data) {
	if (fops == NULL)
		return -EINVAL;

	struct file *f = malloc(sizeof(struct file));
	if (f == NULL)
		return -ENOMEM;

	f->ops = fops;
	f->data = data;
	f->refcount = 1;

	int fd = 0;
	// TODO: Use atomic operations to CAS the pointer. stdatomic doesn't import
	// properly with newlib right now :(
	while (fd < FD_MAX && fds[fd] != NULL) {
		fd++;
	}
	if (fd < FD_MAX) {
		fds[fd] = f;
		return fd;
	} else {
		free(f);
		return -ENOSYS;
	}
}

struct file *fd_file_get(int fd) {
	// TODO: lock the fd before recovering the file and incrementing its refcount
	struct file *f = fds[fd];
	if (f == NULL)
		return NULL;
	f->refcount++;
	return f;
}

void fd_file_put(struct file *file) {
	if (file == NULL)
		return;
	if (--file->refcount == 0) {
		file->ops->release(file);
		free(file);
	}
}

int fd_close(int fd) {
	// TODO: Atomically swap fds[fd] with NULL.
	if (fds[fd] == NULL)
		return -EBADF;
	fd_file_put(fds[fd]);
	fds[fd] = NULL;
	return 0;
}

int dup2(int oldfd, int newfd) {
	struct file *f;

	// If oldfd == newfd, we need to return newfd without closing it.
	if (oldfd == newfd)
		return newfd;

	// TODO: Lock newfd first, close it, set it to the oldfd's file, and unlock
	fd_close(newfd);
	f = fd_file_get(oldfd);
	fds[newfd] = f;
	return newfd;
}
