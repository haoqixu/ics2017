#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
  int n;
  if (args == NULL || sscanf(args, "%i", &n) != 1)
    n = 1;
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  int i;
  char *subcmd;

  if ((subcmd = strtok(NULL, " ")) == NULL) {
    printf("Nothing to do.\n");
  } else if (strcmp(subcmd, "r") == 0) {
    for (i = R_EAX; i <= R_EDI; i++)
      printf("%s\t0x%08x\n", regsl[i], reg_l(i));
    printf("eip\t0x%08x\n", cpu.eip);
  } else if (strcmp(subcmd, "w") == 0) {
    print_wp();
  } else {
    printf("Unknown subcommand: %s\n", subcmd);
  }

  return 0;
}

static int cmd_x(char *args) {
  int n;
  bool success;
  vaddr_t addr;

  char *arg_n = strtok(NULL, " ");
  char *arg_addr = arg_n + strlen(arg_n) + 1;

  if (arg_addr == NULL || *arg_addr == '\0'
      || arg_n == NULL || sscanf(arg_n, "%i", &n) != 1 || n < 1)
    goto err;

  addr = expr(arg_addr, &success);
  if (!success)
    goto err;

  while (n--) {
    printf("0x%08x: 0x%08x\n", addr, vaddr_read(addr, 4));
    addr += 4;
  }
  return 0;

err:
  printf("Invalid command.\n");
  return 0;
}

static int cmd_p(char *args) {
  bool success;
  uint32_t result;
  if (args)
    result = expr(args, &success);
  else
    goto err;

  if (success) {
    printf("%d\n", result);
    return 0;
  }

err:
  printf("Invalid expression\n");
  return 0;
}

static int cmd_w(char *args) {
  bool success;
  uint32_t val;

  if (args == NULL)
    goto err;

  val = expr(args, &success);
  if (!success)
    goto err;

  WP *wp = new_wp();
  wp->expr = strdup(args);
  wp->old = val;
  printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
  return 0;

err:
  printf("Invalid expression.\n");
  return 0;
}

static int cmd_d(char *args) {
  int n;
  WP *wp;

  if (args == NULL || sscanf(args, "%i", &n) != 1) {
    printf("Invalid watchpoint number: \'%s\'.", args);
    return 0;
  }
  wp = find_wp(n);
  if (wp == NULL) {
    printf("Watchpoint %d doesn't exist.\n", n);
    return 0;
  }
  free_wp(wp);
  printf("Watchpoint %d is deleted.\n", n);
  return 0;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step [N] instruction exactly.", cmd_si },
  { "info", "[r] List registers; [w] List watchpoints.", cmd_info },
  { "x", "Examine the contents of memory.", cmd_x },
  { "p", "Print the value of the expression", cmd_p},
  { "w", "Watchpoint", cmd_w},
  { "d", "Delete watchpoint", cmd_d},

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
