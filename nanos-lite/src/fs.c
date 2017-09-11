#include "fs.h"

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

ssize_t fs_write(int fd, uint8_t *buf, size_t len) {
  size_t i = 0;
  Finfo *fp = &file_table[fd];

  switch (fd) {
  case FD_STDOUT:
  case FD_STDERR:
    while (i++ < len)
      _putc(*buf++);
    return len;

  default:
    if (fd < 6 || fd >= NR_FILES)
      return -1;
    assert(len <= fp->size - fp->open_offset);
    ramdisk_write(buf, fp->disk_offset + fp->open_offset, len);
    fp->open_offset += len;
    return len;
  }
}

int fs_open(const char *pathname, int flags, int mode) {
  (void)flags; (void)mode; /* UNUSED */
  int fd;
  for (fd = 0; fd < NR_FILES; fd++)
    if (strcmp(pathname, file_table[fd].name) == 0)
      break;
  assert(fd != NR_FILES);
  file_table[fd].open_offset = 0;
  return fd;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  ssize_t size, nread;
  Finfo *fp = &file_table[fd];

  switch (fd) {
  case FD_STDOUT:
  case FD_STDERR:
    return -1;

  default:
    if (fd < 6 || fd >= NR_FILES)
      return -1;
    size = fp->size - fp->open_offset;
    nread = len > size ? size : len;
    ramdisk_read(buf, fp->disk_offset + fp->open_offset, nread);
    fp->open_offset += nread;
    return nread;
  }
}

off_t fs_lseek(int fd, off_t offset,int whence) {
  Finfo *fp = &file_table[fd];

  if (fd < 6 || fd > NR_FILES)
    return 0;

  switch (whence) {
  case SEEK_END: offset = fp->size + offset; break;
  case SEEK_CUR: offset = fp->open_offset + offset; break;
  case SEEK_SET: break;
  default: return -1;
  }

  if (offset > fp->size || offset < 0)
    return -1;

  return fp->open_offset = offset;
}

int fs_close(int fd) {
  (void)fd;
  return 0;
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}
