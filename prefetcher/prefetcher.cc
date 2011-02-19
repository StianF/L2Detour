/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"

                   /* 8 kB maximum memory use: */
struct RPT {
  Addr pc;         /* 8 bytes */
  Addr mem_addr;   /* 8 bytes */
  int diff;        /* 4 bytes on this arch? */
  int diff2;       /* 4 bytes? */
};                 /* = 24 bytes */

const int l = 110; /* times */
RPT rpttable[l];   /* = 2.64 kB if k = 1000 */
int length = 0;
int far = 0;

void prefetch_init(void)
{
  /* Called before any calls to prefetch_access. */
  /* This is the place to initialize data structures. */

  //DPRINTF(Leif, "Initialized sequential-on-happy prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
  Addr fetch = 0;
  bool found = false;
  int i = 0;

  // Traverse table for matching entry and if match calculate next fetch.
  for (i = 0; i < far; i++) {
    if (rpttable[i].pc == stat.pc) {
      found = true;
      if (rpttable[i].diff == 0) {
        rpttable[i].diff = stat.mem_addr - rpttable[i].mem_addr;
        rpttable[i].mem_addr = stat.mem_addr;
      } else {
        // RPT-ish.
        if (rpttable[i].diff2 == rpttable[i].diff) {
          fetch = stat.mem_addr+rpttable[i].diff;
        } else if(rpttable[i].diff2 != 0) {
          // This causes the prefetcher to
          // remember what diff2 was last time around and continues
          // to try to match that. FIXME?
          rpttable[i].diff = 0;
          rpttable[i].mem_addr = stat.mem_addr;
        } else {
          rpttable[i].diff2 = stat.mem_addr - rpttable[i].mem_addr;
        }
      }
    }
  }

  // Add non-existing entry to table.
  if (!found) {
    if (length < l-1) {
      length = length + 1;
      if (far != l) {
        far = far +1;
      }
    } else {
      length = 0;
    }

    rpttable[length].pc = stat.pc;
    rpttable[length].mem_addr = stat.mem_addr;
    /* If diff equal to zero than same element used twice,
       probably no need for the prefetcher to do anything.
       Therefore can use it as the guard value. */
    rpttable[length].diff = 0;
    rpttable[length].diff2 = 0;
  }

  if (fetch) {
    // Fetch the strided address if block not in cache, if in cache
    // fetch next block.
    if (MAX_PHYS_MEM_ADDR >= fetch &&
        !(in_cache(fetch) || in_mshr_queue(fetch))) {
      issue_prefetch(fetch);
    } else if (MAX_PHYS_MEM_ADDR >= fetch + BLOCK_SIZE &&
        !(in_cache(fetch + BLOCK_SIZE) || in_mshr_queue(fetch + BLOCK_SIZE))) {
      issue_prefetch(fetch + BLOCK_SIZE);
    }
  } else if (stat.miss) {
    // On miss and not in table, make sure that some following blocks
    // are either being prefetched, fetched or is available.
    int i;
    fetch = stat.mem_addr;
    for (i = 1; i < 4; i++) {
      fetch += BLOCK_SIZE;
      if (MAX_PHYS_MEM_ADDR >= fetch &&
          !(in_cache(fetch) || in_mshr_queue(fetch))) {
        issue_prefetch(fetch);
      }
    }
  } else {
    // On access, a distance of four cache blocks gave good performance in
    // the shortest loop-test (accumulate).
    fetch = stat.mem_addr + BLOCK_SIZE * 4;

    if (MAX_PHYS_MEM_ADDR >= fetch &&
        !(in_cache(fetch) || in_mshr_queue(fetch))) {
      issue_prefetch(fetch);
    }
  }
}

void prefetch_complete(Addr addr) {
}
