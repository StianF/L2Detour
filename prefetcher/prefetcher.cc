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
  node_t *parent;
  node_t *child;
  node_t *sibling;
} node_t;

typedef struct root {
  Addr pc;
  node_t *child;

  // The node in the current prefetch path known to be good.
  Addr last;
  node_t *cur;
  int cur_count;

  // The node upto which we have prefetched. 
  node_t *pf_node;
  int pf_count;
  char pf_learning;
} root_t;

// Maybe a bit large penalty in case of wrong guessing in learning phase.
#define DEFAULT_COUNT 4

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

      // Need to cleanup the root before reuse? FIXME
    }

    root->child = malloc(sizeof(node));
    root->child->delta = 1;
    root->child->count = DEFAULT_COUNT;
    root->cur = root->child;
    root->pf_node = root->child;
    root->pf_learning = 1; // TRUE
    root->pf_count = 0;
  }

  // Check if on valid path
  if (stat.mem_addr == root->last + root->cur->delta) {
    if (root->cur_count++ == root->cur->count) {
      if (root->pf_learning) {
        // While learning, assume the current stride is still a good guess.
        root->cur->count++;
      } else {
        root->cur_count = 0;
        root->cur = root->cur->child;

        // If current node is leaf, go to root.
        // TODO choose the most likely root child.
        if (!(root->cur)) root->cur = root->child;
      }
    }
  } else {
    // Have not guessed correctly.
    if (root->pf_learning) {
      // The current count is the seen number of times access has been
      // with this stride. Set the node to this value for future guessing.
      root->cur->count = root->cur_count; 

      // FIXME the new stride should be used as a criteria for branching
      // back to root and perhaps enable learning?

      // If the startup nodes stride is wrong (i.e. stride = 1 gives no counts)
      // reuse the node instead of storing a zero count node.
      int stride = stat.addr - root->last;
      if (!root->cur->count) {
        root->cur->stride = stride;
      } else {
        // Add a child and give it the stride as in true-case.

        // TODO: Check if child has the stride, in which case traverse this
        // first and potentially split it if count not the same (perhaps
        // allow for some slack in the requirement for splitting).

        // FIXME nonchalently overwrites any child already present.
        root->cur->child = malloc(sizeof(node));
        root->cur->child->parent = root->cur;
        root->cur = root->cur->child;
        root->cur_count = 1;

        root->cur->delta = stride;
        root->cur->count = DEFAULT_COUNT;
      }
    }
  }

  root->last = stat.mem_addr;

  // Prefetch

  // Find the count of locations the prefetcher is ahead of the
  // verified current data access.
  int ahead = root->pf_count;
  node *aheadp = root->pf_node;
  while (aheadp != root->cur) {
    aheadp = aheadp->parent;
    if (!aheadp) break;
    ahead += aheadp->count;
  }

  while (ahead < 4) { // FIXME Magic number.
    if (root->pf_count < root->pf_node->count) {
      // FIXME only prefetches addresses equal to the stride.
      issue_prefetch(addr);
      ahead++;
      root->pf_count++;
    } else if (root->pf_node->child) {
      root->pf_node = root->pf_node->child;
      root->pf_count = 0;
    } else {
      break;
      /*
       * Would make it impossible to parent my way up to the cur-node as
       * needed in the logic-paragraph above. FIXME
       *
      root->pf_node = root->child;
      root->pf_count = 0;
      */
    }
  }
}

void prefetch_complete(Addr addr) {
  /*
   * Called when a block requested by the prefetcher has been loaded.
   */
}
