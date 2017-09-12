#include "fs.h"

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);
void dispinfo_read(void *buf, off_t offset, size_t len);
size_t events_read(void *buf, size_t len);

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
  file_table[FD_FB].size = _screen.width * _screen.height * sizeof(uint32_t);
  file_table[FD_FB].open_offset = 0;
}

ssize_t fs_write(int fd, uint8_t *buf, size_t len) {
  size_t i = 0;
  size_t size, nwrite;
  Finfo *fp = &file_table[fd];

  size = fp->size - fp->open_offset;
  nwrite = len > size ? size : len;

  switch (fd) {
  case FD_STDOUT:
  case FD_STDERR:
    while (i++ < len)
      _putc(*buf++);
    return len;

  case FD_FB:
    fb_write(buf, fp->open_offset, nwrite);
    break;

  default:
    if (fd < 6 || fd >= NR_FILES)
      return -1;
    ramdisk_write(buf, fp->disk_offset + fp->open_offset, nwrite);
    break;
  }
  fp->open_offset += nwrite;
  return nwrite;
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

  size = fp->size - fp->open_offset;
  nread = len > size ? size : len;


  switch (fd) {
  case FD_STDOUT:
  case FD_STDERR:
    return -1;

  case FD_DISPINFO:
    dispinfo_read(buf, fp->open_offset, nread);
    break;

  case FD_EVENTS:
    return events_read(buf, len);

  default:
    if (fd < 6 || fd >= NR_FILES)
      return -1;
    ramdisk_read(buf, fp->disk_offset + fp->open_offset, nread);
    break;
  }
  fp->open_offset += nread;
  return nread;
}

off_t fs_lseek(int fd, off_t offset,int whence) {
  Finfo *fp = &file_table[fd];

  assert(fd < NR_FILES);

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
