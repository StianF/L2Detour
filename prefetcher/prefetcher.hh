#include "interface.hh"

typedef struct node {
  Addr delta;
  struct node *parent;
  struct node *child;
  struct node *sibling;
} node_t;

typedef struct root {
  Addr pc;
  node_t *child;
  int num_nodes;

  // The node in the current prefetch path known to be good.
  Addr last;
  node_t *cur;

  // The node upto which we have prefetched. 
  node_t *pf_node;
  char pf_learning;
  int ahead; // The number of addresses ahead of validated.
} root_t;
