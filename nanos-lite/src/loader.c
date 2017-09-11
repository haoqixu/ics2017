#include "common.h"
#include "fs.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  size_t nbyte = fs_filesz(fd);

  Log("loaded: [%d]%s size:%d", fd, filename, nbyte);

  fs_read(fd, DEFAULT_ENTRY, nbyte);

  return (uintptr_t)DEFAULT_ENTRY;
}
