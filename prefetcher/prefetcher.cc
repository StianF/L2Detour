/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
*/

#include "interface.hh"

#include <stdio.h>
#include <stdlib.h>

typedef struct node {
  Addr delta;
  struct node *parent;
  struct node *child;
  struct node *sibling;
} node_t;

typedef struct root {
  Addr pc;
  node_t *child;

  // The node in the current prefetch path known to be good.
  Addr last;
  node_t *cur;

  // The node upto which we have prefetched. 
  node_t *pf_node;
  char pf_learning;
  int ahead; // The number of addresses ahead of validated.
} root_t;

// Maybe a bit large penalty in case of wrong guessing in learning phase.
#define DEFAULT_COUNT 4
#define MAX_STRIDE 1024

#define NUM_ROOTS 100
root_t *roots[NUM_ROOTS + 1];
int rr_count = 0;

void prefetch_init(void)
{
  /* Called before any calls to prefetch_access. */
  /* This is the place to initialize data structures. */
  //DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");

  roots[NUM_ROOTS] = 0; // Guard value, think C string. Should be constant.
  int f;
  for (f = 0; f < NUM_ROOTS; f++)
    roots[f] = (root_t *)malloc(sizeof(root_t));
}

void print_child_path(node_t *r, node_t *cur, node_t *pf) {
  while (r) {
    if (r == pf) printf("p");
    if (r == cur) printf("c");
    printf("%i ", r->delta);
    if (r->child && r->child->parent != r) printf("!");
    r = r->child;
  }
  printf("\n");
}

void prefetch_access(AccessStat stat) {
  printf("Mem addr: %i\n", stat.mem_addr);
  // Search for matching root.
  root_t **rootp = roots;
  root_t *root = *rootp;
  while (root) {
    if (root->pc == stat.pc) break; // Found a match.
    root = *++rootp;
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

      // Need to cleanup the root before reuse? FIXME
    }

    root->pc = stat.pc;
    root->pf_learning = 1; // TRUE
    printf("learning\n");

    /*
    root->child = (node_t *)malloc(sizeof(node_t));
    root->child->delta = 1;
    root->cur = root->child;
    root->pf_node = root->cur;

    // Faking this since first access is a special case of always valid path.
    root->last -= root->cur->delta;
    root->ahead = 1;
    */
  }

  print_child_path(root->child, root->cur, root->pf_node);

  printf("mem_addr: %i, last: %i, cur->delta: %i\n",
         (int)stat.mem_addr, (int)root->last, root->cur?root->cur->delta:0);

  int stride = stat.mem_addr - root->last;
  // Check if on valid path
  if (root->cur && stride == root->cur->delta) {
    printf("correct\n");
    root->ahead--;
    if (!root->pf_learning) {
      root->cur = root->cur->child;

      // If current node is leaf, go to root.
      // TODO choose the most likely root child.
      // FIXME, doesn't count the correct number of times for first.
      if (!(root->cur)) root->cur = root->child;
    }
  } else {
    // Have not guessed correctly.
    printf("wrong\n");

    root->ahead = 0;

    // If learning and we are guessing too large addresses, stop the
    // learning mode and try to prefetch from root. This makes it so
    // that only increasing patterns are stored in a path. FIXME make
    // an adjustment so the path knows if it's increasing or decreasing
    // -- the prefetcher should be able to accommodate both prefetch
    // directions.
    if (root->pf_learning && root->cur && stride < root->cur->delta) {
      root->pf_learning = 0;
      printf("traversing\n");

      // Parent could be avoided here, just traverse until match found.
      root->pf_node->parent->child = NULL;

      // TODO Guess correct path/restart learning if matching branch found.

      // Have already verified the first by guessing path
      // (or begun learning)
      if (root->child->child) root->cur = root->child->child;
      else root->cur = root->child;
      root->pf_node = root->cur;
    }
  }

  if (root->pf_learning) {
    if (!root->cur) {
      root->cur = (node_t *)malloc(sizeof(node_t));
      root->child = root->cur;
      root->pf_node = root->cur;
      root->cur->delta = 1;
    } else {
      root->cur->delta = stride;
      // Add a child and give it the stride as in true-case.

      // TODO: Check if child has the stride, in which case traverse this
      // first and potentially split it if count not the same (perhaps
      // allow for some slack in the requirement for splitting).

      // FIXME nonchalently overwrites any child already present.
      root->cur->child = (node_t *)malloc(sizeof(node_t));
      root->cur->child->parent = root->cur;
      root->cur = root->cur->child;
      root->pf_node = root->cur;
      root->cur->delta = stride;
    }
  }

  root->last = stat.mem_addr;

  // Prefetch

  /*
  printf("pf_count: %i\n", root->pf_count);
  printf("cur_count: %i\n", root->cur_count);
  printf("ahead: %i\n", ahead);
  */

  print_child_path(root->child, root->cur, root->pf_node);

  while (root->ahead < 4) { // FIXME Magic number.
    // FIXME only prefetches stride as addresses.
    issue_prefetch(root->pf_node->delta);
    root->ahead++;

    root->pf_node = root->pf_node->child;
    if (!root->pf_node) root->pf_node = root->child;
  }

  printf("\n");
}

void prefetch_complete(Addr addr) {
  /*
   * Called when a block requested by the prefetcher has been loaded.
   */
}
