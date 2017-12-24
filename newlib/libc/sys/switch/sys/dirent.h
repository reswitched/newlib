/* libc/sys/linux/sys/dirent.h - Directory entry as returned by readdir */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#include<libtransistor/fs/inode.h>

#define _DIRENT_HAVE_D_NAMLEN
#define MAXNAMLEN 255		/* sizeof(struct dirent.d_name)-1 */

struct dirent {
	ino_t d_ino;
	size_t d_namlen;
	char d_name[256];
};

typedef struct {
	trn_dir_t dir;
	struct dirent ent;
} DIR;

DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int readdir_r(DIR *__restrict, struct dirent *__restrict,
              struct dirent **__restrict);
void rewinddir(DIR *);
int closedir(DIR *);

//#ifndef _POSIX_SOURCE
#if false
long telldir (DIR *);
void seekdir (DIR *, off_t loc);

int scandir (const char *__dir,
             struct dirent ***__namelist,
             int (*select) (const struct dirent *),
             int (*compar) (const struct dirent **, const struct dirent **));

int alphasort (const struct dirent **__a, const struct dirent **__b);
#endif /* _POSIX_SOURCE */

#endif
