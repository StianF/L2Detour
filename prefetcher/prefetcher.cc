/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
*/

#include "interface.hh"

void prefetch_init(void)
{
  /* Called before any calls to prefetch_access. */
  /* This is the place to initialize data structures. */
  DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

typedef struct node {
  Addr delta;
  int count;
  node_t *child;
  node_t *sibling;
} node_t;

typedef struct pf {
  node_t *node;
  int count;
  char learning;
} pf_t;

typedef struct root {
  Addr pc;
  node_t *cur;
  node_t *child;
  pf_t pf;
} root_t;

#define NUM_ROOTS 100
root_t roots[NUM_ROOTS + 1];
roots[NUM_ROOTS] = 0;
int rr_count = 0;

void prefetch_access(AccessStat stat) {
  // Search for matching root.
  node_t *root = roots;
  while (root) {
    if (root->pc == stat.pc) break; // Found a match.
    root++;
  }

  // Didn't find a match, add one.
  if (!root) {
    // root is actually now an empty slot. If not guard value, use it.
    // If guard value need to choose a slot in use.
    if (root == roots[NUM_ROOTS]) {
      // TODO: Throw out according to a suitable replacement policy (LRU?).
      // For now just use round-robin.
      root = roots[rr_count++];
      if (rr_count == NUM_ROOTS) rr_count = 0;
    }

    root->child = malloc(sizeof(node));
    root->child->delta = 1;
    root->child->count = 4;
    root->pf->node = root->child;
    root->pf->learning = 1; // TRUE
  }
}

void prefetch_complete(Addr addr) {
  /*
   * Called when a block requested by the prefetcher has been loaded.
   */
}
