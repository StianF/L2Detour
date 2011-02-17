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
};                 /* = 20 bytes */

const int l = 130; /* times */
RPT rpttable[l];   /* = 2.6 kB if k = 1000 */
int length = 0;
int far = 0;

void prefetch_init(void)
{
  /* Called before any calls to prefetch_access. */
  /* This is the place to initialize data structures. */

  DPRINTF(Leif, "Initialized sequential-on-happy prefetcher\n");
}

bool first = true;
bool second = false;

void prefetch_access(AccessStat stat)
{
  Addr fetch = 0;
  bool found = false;
  int i = 0;

  // Traverse table for matching entry.
  for (i = 0; i < far; i++) {
    if (rpttable[i].pc == stat.pc) {
      found = true;
      if (rpttable[i].diff == 0) {
        rpttable[i].diff = stat.mem_addr - rpttable[i].mem_addr;
        rpttable[i].mem_addr = stat.mem_addr;
      } else {
        fetch = stat.mem_addr+rpttable[i].diff;
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
  } else if (fetch != 0 && MAX_PHYS_MEM_ADDR > fetch) {
    if (!in_cache(fetch)) {
      issue_prefetch(fetch);
    }
  }/*else
     {
     int next = 4;
     while(in_cache(stat.mem_addr + (BLOCK_SIZE * next)))
     {
     next = next +1;
     }
     issue_prefetch(stat.mem_addr + (BLOCK_SIZE * next));
     }*/
}

void prefetch_complete(Addr addr) {
  /*
   * Called when a block requested by the prefetcher has been loaded.
   */
}
