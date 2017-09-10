#include "common.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  size_t nbyte = get_ramdisk_size();
  ramdisk_read(DEFAULT_ENTRY, 0, nbyte);

  return (uintptr_t)DEFAULT_ENTRY;
}
