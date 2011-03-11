#include "interface.hh"
#include <stdio.h>

int main(void) {
  AccessStat s = {642, 0, 0, 0};
  prefetch_access(s);
}

void issue_prefetch(Addr addr) {
  printf("Prefetched: %i\n", addr);
}
