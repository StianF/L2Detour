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
bool fourth = false;

void prefetch_access(AccessStat stat)
{
  if (second) {
    DPRINTF(Leif, "Second access\n");
    DPRINTF(Leif, "Is addr 100 in cache? %i\n", in_cache(100));
    DPRINTF(Leif, "Is addr 100 in mshr? %i\n", in_mshr_queue(100));
    DPRINTF(Leif, "Is addr 101 in cache? %i\n", in_cache(101));
    DPRINTF(Leif, "Is addr 101 in mshr? %i\n", in_mshr_queue(101));
    DPRINTF(Leif, "Is addr 102 in cache? %i\n", in_cache(102));
    DPRINTF(Leif, "Is addr 102 in mshr? %i\n", in_mshr_queue(102));
    second = false;
  }

  if (first) { // Testing shit out.
    DPRINTF(Leif, "First access\n");
    DPRINTF(Leif, "Is addr 100 in cache? %i\n", in_cache(100));
    DPRINTF(Leif, "Is addr 100 in mshr? %i\n", in_mshr_queue(100));
    DPRINTF(Leif, "Is addr 101 in cache? %i\n", in_cache(101));
    DPRINTF(Leif, "Is addr 101 in mshr? %i\n", in_mshr_queue(101));
    DPRINTF(Leif, "Is addr 102 in cache? %i\n", in_cache(102));
    DPRINTF(Leif, "Is addr 102 in mshr? %i\n", in_mshr_queue(102));
    DPRINTF(Leif, "Issuing prefetch of addr 100!\n");
    issue_prefetch(100);
    DPRINTF(Leif, "Is addr 100 in cache? %i\n", in_cache(100));
    DPRINTF(Leif, "Is addr 100 in mshr? %i\n", in_mshr_queue(100));
    DPRINTF(Leif, "Is addr 101 in cache? %i\n", in_cache(101));
    DPRINTF(Leif, "Is addr 101 in mshr? %i\n", in_mshr_queue(101));
    DPRINTF(Leif, "Is addr 102 in cache? %i\n", in_cache(102));
    DPRINTF(Leif, "Is addr 102 in mshr? %i\n", in_mshr_queue(102));

    first = false;
    second = true;
  }

  if (fourth) {
    DPRINTF(Leif, "First access after completed prefetch\n");
    DPRINTF(Leif, "Is addr 100 in cache? %i\n", in_cache(100));
    DPRINTF(Leif, "Is addr 100 in mshr? %i\n", in_mshr_queue(100));
    DPRINTF(Leif, "Is addr 101 in cache? %i\n", in_cache(101));
    DPRINTF(Leif, "Is addr 101 in mshr? %i\n", in_mshr_queue(101));
    DPRINTF(Leif, "Is addr 102 in cache? %i\n", in_cache(102));
    DPRINTF(Leif, "Is addr 102 in mshr? %i\n", in_mshr_queue(102));
    fourth = false;
  }

  DPRINTF(Leif, "Current queue size: %i\n", current_queue_size());

  #if 0
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
  }

  if (fetch != 0 && MAX_PHYS_MEM_ADDR > fetch) {
    if (!in_cache(fetch)) {
      issue_prefetch(fetch);
    }
  } else if (stat.miss) {
    // On miss and not in table, make sure that some following blocks
    // are being prefetched.
    
  } else {
    // On access, a distance of four cache blocks gave good performance in
    // the shortest loop-test (accumulate).
  }
  #endif

}

bool third = true;

void prefetch_complete(Addr addr) {
  if (third) { // Testing shit out.
    DPRINTF(Leif, "Completed prefetch!\n");
    DPRINTF(Leif, "Current queue size: %i\n", current_queue_size());
    DPRINTF(Leif, "Is addr 100 in cache? %i\n", in_cache(100));
    DPRINTF(Leif, "Is addr 100 in mshr? %i\n", in_mshr_queue(100));
    DPRINTF(Leif, "Is addr 101 in cache? %i\n", in_cache(101));
    DPRINTF(Leif, "Is addr 101 in mshr? %i\n", in_mshr_queue(101));
    DPRINTF(Leif, "Is addr 102 in cache? %i\n", in_cache(102));
    DPRINTF(Leif, "Is addr 102 in mshr? %i\n", in_mshr_queue(102));
    third = false;
    fourth = true;
  }
}
