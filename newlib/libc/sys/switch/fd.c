#include <errno.h>
#include <stdlib.h>

#include "fd.h"

#define FD_MAX 1024
static _Atomic(struct file *)fds[FD_MAX] = {ATOMIC_VAR_INIT(NULL)};

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
	while (fd < FD_MAX && atomic_compare_exchange_weak(&fds[fd], NULL, f)) {
		fd++;
	}
	if (fd < FD_MAX) {
		return fd;
	} else {
		free(f);
		return -ENOSYS;
	}
}

struct file *fd_file_get(int fd) {
	struct file *f = fds[fd];
	if (f == NULL)
		return NULL;
	// TODO: This doesn't work. If, between acquiring f and adding 1, the file
	// is freed, we'll blow up ! We'd need a way to avoid the null-check, so
	// we could add first, and acquire f afterward...
	if (atomic_fetch_add(&f->refcount, 1) == 1) {
		// The file was decremented, and will probably be freed soon...
		return NULL;
	}
	return f;
}

void fd_file_put(struct file *file) {
	if (file == NULL)
		return;

	// If we're the last to abandon our pointer, we're responsible for
	// destroying it.
	if (atomic_fetch_sub(&file->refcount, 1) == 1) {
		file->ops->release(file);
		free(file);
	}
}

int fd_close(int fd) {
	// First, make sure the file is unreachable from this point on.
	struct file *file = atomic_exchange(&fds[fd], NULL);
	if (file == NULL)
		return -EBADF;

	// Then, actually release the file
	fd_file_put(file);
	return 0;
}

int dup2(int oldfd, int newfd) {
	struct file *f;
	struct file *old;

	// If oldfd == newfd, we need to return newfd without closing it.
	if (oldfd == newfd)
		return newfd;

	// Acquire the old file
	f = fd_file_get(oldfd);

	// "Close" and duplicate our file.
	old = atomic_exchange(&fds[newfd], f);

	// Actually free the old file if necessary.
	if (old != NULL)
		fd_file_put(old);

	return newfd;
}
