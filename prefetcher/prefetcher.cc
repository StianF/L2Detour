/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
struct RPT {
    Addr pc;        // Program Counter, used to index(ish). 
    Addr mem_addr;  // Last memory addr request. 
    int diff;       // Difference between mem_addr in table and request addr. 
    int priority;   // Priority for throw-out scheme. LRU
};
const int l = 130;
RPT rpttable[l];

int length = 0;
int far = 0;
void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-happy prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
		Addr fetch = 0;
		bool found = false;
		int i = 0;

        int lru_index = 0, lru_max = 0;
 
        // Sequential search for PC in RPT-table.
		for(i = 0; i < far; i++){
			if(rpttable[i].pc == stat.pc){
				found = true;
				if(rpttable[i].diff == 99999){
					rpttable[i].diff = stat.mem_addr - rpttable[i].mem_addr;
					rpttable[i].mem_addr = stat.mem_addr;
				}else{
					fetch = stat.mem_addr+rpttable[i].diff;
				}
                rpttable[i].priority = 0;
			}
            // Increase priority of all other, and save max' index.
            else {
                rpttable[i].priority++;
                if ( rpttable[i].priority > lru_max )
                    lru_index = i;
            }
		}
        // If it is not in the table, add PC to table
        // LRU throw-out scheme.
		if(!found){
			if(length < l-1){
				length = length + 1;
				if(far != l){
					far = far +1;
				}

			}else{
				length = 0;
			}
				
			RPT rpt;
			rpt.pc = stat.pc;
			rpt.mem_addr = stat.mem_addr;
			rpt.diff = 99999;
			rpttable[lru_index] = rpt;
		}
        // If found, prefetch memory if needed.
		else if(fetch != 0 && MAX_PHYS_MEM_ADDR > fetch){
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

