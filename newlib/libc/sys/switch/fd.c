#include <errno.h>
#include <stdlib.h>
#include <libtransistor/nx.h>
#include "fd.h"

#define FD_MAX 1024
#define IS_VALID(fd) fd >= 0 && fd < FD_MAX
struct fd {
	atomic_int lock;
	_Atomic(struct file *)file;
};

static struct fd fds[FD_MAX] = {{
	.lock = ATOMIC_VAR_INIT(0),
	.file = ATOMIC_VAR_INIT(NULL)
}};

static void lock_fd(struct fd *fd) {
	int expected = 0;
	while (!atomic_compare_exchange_strong(&fd->lock, &expected, 1)) {
		// Yield
		svcSleepThread(0);
	}
}

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
	struct file *expected = NULL;
	while (fd < FD_MAX && atomic_compare_exchange_strong(&fds[fd].file, &expected, f)) {
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
	if (!IS_VALID(fd)) {
		return NULL;
	}

	// First, acquire the lock for this fd
	lock_fd(&fds[fd]);

	// The fd is locked. We can now acquire the underlying file
	struct file *f = fds[fd].file;
	if (f == NULL)
		goto end;
	// And increment its counter
	f->refcount++;
end:
	// Before releasing the lock
	fds[fd].lock = 0;
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
	if (!IS_VALID(fd)) {
		return -EBADF;
	}

	// First, make sure we lock the fd
	lock_fd(&fds[fd]);

	// Then, make sure the file is unreachable from this point on.
	struct file *file = atomic_exchange(&fds[fd].file, NULL);
	if (file == NULL)
		return -EBADF;

	// We can now give up the lock
	fds[fd].lock = 0;

	// Then, actually release the file
	fd_file_put(file);
	return 0;
}

int dup2(int oldfd, int newfd) {
	struct file *f;
	struct file *old;

	if (!IS_VALID(oldfd) || !IS_VALID(newfd)) {
		errno = EBADF;
		return -1;
	}

	// If oldfd == newfd, we need to return newfd without closing it.
	if (oldfd == newfd)
		return newfd;

	// Acquire the old file
	f = fd_file_get(oldfd);
	if (f == NULL) {
		errno = EBADF;
		return -1;
	}

	// Then, lock the fd
	lock_fd(&fds[newfd]);

	// "Close" and duplicate our file.
	old = atomic_exchange(&fds[newfd].file, f);

	// We can release the lock now
	fds[newfd].lock = 0;

	// Actually free the old file if necessary.
	if (old != NULL)
		fd_file_put(old);

	return newfd;
}
