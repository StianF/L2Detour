#include <stdio.h>
#include "prefetcher.hh"

void print_child_path(node_t *r, node_t *cur, node_t *pf);

int main(void) {
  // Testing if printing path with branches work.
  node_t n1, n2, n3, n5;
  n1.delta = 1;
  n2.delta = 2;
  n3.delta = 3;
  n5.delta = 5;

  n1.sibling = NULL;
  n2.sibling = NULL;
  n5.sibling = NULL;

  n1.child = &n2;
  n2.child = &n3;
  n3.sibling = &n5;
  n3.child = NULL;

  print_child_path(&n1, NULL, NULL);

  return 0;
}

void issue_prefetch(Addr addr) {
}
