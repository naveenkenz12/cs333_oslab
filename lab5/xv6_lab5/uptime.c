//new_adding_start
#include "types.h"
#include "stat.h"
#include "user.h"
#include "syscall.h"
#include "param.h"
#include "types.h"
#include "fs.h"
#include "fcntl.h"
#include "traps.h"
#include "memlayout.h"

int
main(void)
{
  int stdout = 1;
  int i=uptime();
  printf(stdout, "\nuptime: %d\n\n",i);
  exit();
}
//new_adding_end