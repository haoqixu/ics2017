#include "common.h"
#include "memory.h"
#include "fs.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  size_t nbyte = fs_filesz(fd);
  void *pa;
  void *va;

  Log("loaded: [%d]%s size:%d", fd, filename, nbyte);

  void *end = DEFAULT_ENTRY + nbyte;
  for (va = DEFAULT_ENTRY; va < end; va += PGSIZE) {
    pa = new_page();
    _map(as, va, pa);
    fs_read(fd, pa, (end - va) < PGSIZE ? (end - va) : PGSIZE);
  }

  return (uintptr_t)DEFAULT_ENTRY;
}
