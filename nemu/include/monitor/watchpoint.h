#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char *expr;
  uint32_t old;


} WP;

void init_wp_pool();
WP *new_wp();
void free_wp(WP *);
bool wp_has_changed();
WP *find_wp(int);
void print_wp();


#endif
