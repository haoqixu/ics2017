#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include <stdlib.h>

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp() {
  Assert(free_, "There is no more WP.");
  WP *t = free_;
  free_ = free_->next;
  t->next = head;
  head = t;
  return t;
}

void free_wp(WP *wp) {
  WP **cur;
  for (cur = &head; *cur; cur = &(*cur)->next) {
    if (*cur == wp) {
      *cur = wp->next;
      wp->next = free_;
      free(wp->expr);
      free_ = wp;
      break;
    }
  }
}

bool wp_has_changed()
{
  WP *p;
  uint32_t new;
  bool success;
  bool has_changed = false;

  for (p = head; p; p = p->next) {
    new = expr(p->expr, &success);
    Assert(success, "invalid expression.");
    if (new != p->old) {
      printf("\n%s:\nOld value = %d\nNew value = %d\n",
        p->expr, p->old, new);
      p->old = new;
      has_changed = true;
    }
  }
  return has_changed;
}

WP *find_wp(int n) {
  WP *p;
  for (p = head; p; p = p->next)
    if (p->NO == n)
      return p;

  return NULL;
}

void print_wp() {
  WP *p; 
  printf("Num\tExpression\tValue\n");
  for (p = head; p; p = p->next)
    printf("%d\t%s\t%u\n", p->NO, p->expr, p->old);
}
