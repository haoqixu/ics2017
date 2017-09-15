#include "common.h"

extern int snprintf(char *s, size_t n, const char *format, ...);

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key_code;

  if ((key_code = _read_key()) == _KEY_NONE) {
    snprintf(buf, len, "t %d\n", _uptime());
  } else if (key_code & 0x8000) {
    key_code ^= 0x8000;
    snprintf(buf, len, "kd %s\n", keyname[key_code]);
    extern void switch_game();
    if (key_code == _KEY_F12)
      switch_game();
  } else {
    snprintf(buf, len, "ku %s\n", keyname[key_code]);
  }
  return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int row, col;
  offset /= 4;
  col = offset % _screen.width;
  row = offset / _screen.width;

  _draw_rect(buf, col, row, len/4, 1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
