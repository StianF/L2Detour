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
  Addr pc;
  node_t *child;
  node_t *sibling;
} node_t;

#define NUM_ROOTS 100
node_t *roots[NUM_ROOTS + 1];
roots[NUM_ROOTS] = 0;

void prefetch_access(AccessStat stat) {
  // Search for matching root.
  node_t *root = roots;
  while (root) {
    if (root->pc != stat.pc) break; // Found a match.
    root++;
  }

  // Didn't find a match, add one.
  if (!root) {
    // root is actually now an empty slot. If not memguard, use it.
    if (root == roots[NUM_ROOTS]) {
      // Throw out the LRU?
    } else {
      // Use the empty slot
    }
  }
}

void prefetch_complete(Addr addr) {
  /*
   * Called when a block requested by the prefetcher has been loaded.
   */
}
