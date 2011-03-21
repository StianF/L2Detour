#include "interface.hh"
#include <stdio.h>

int main(void) {
  struct AccessStat s = {642, 0, 0, 0};
  prefetch_init();
  prefetch_access(s);

  s.mem_addr += 1;
  prefetch_access(s);
  s.mem_addr += 2;
  prefetch_access(s);
  s.mem_addr += 3;
  prefetch_access(s);

  s.mem_addr += 1;
  prefetch_access(s);
  s.mem_addr += 2;
  prefetch_access(s);
  s.mem_addr += 5;
  prefetch_access(s);
  s.mem_addr += 7;
  prefetch_access(s);

  s.mem_addr += 5;
  prefetch_access(s);

  /*
  s.mem_addr += 1;
  prefetch_access(s);
  s.mem_addr += 2;
  prefetch_access(s);
  s.mem_addr += 3;
  prefetch_access(s);
  */
}

void issue_prefetch(Addr addr) {
  printf("Prefetched: %i\n", addr);
}
