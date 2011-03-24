/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"

#include "nval.h"

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
  Addr pf_addr = stat.mem_addr;

  int i;
  for (i = 0; i < N_VAL; i++) {
    pf_addr += BLOCK_SIZE;
  
    if (!in_cache(pf_addr)) && !in_mshr_queue(pf_addr))
      issue_prefetch(pf_addr);
  }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
