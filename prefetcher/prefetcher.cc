/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
struct MU {	
	int diff;
	int times_used;
};	

struct RPT {
    Addr pc;        
		Addr mem_addr; 
    int diff;     
		int diff2;
		MU stats[3];
};

const int l = 100;
RPT rpttable[l];


int length = 0;
int far = 0;
void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
		Addr fetch = 0;
		bool found = false;
		int i = 0;
		int found_index = 0;
		for(i = 0; i < far; i++){
			if(rpttable[i].pc == stat.pc){
				found = true;
				if(rpttable[i].diff == 99999){
					rpttable[i].diff = stat.mem_addr - rpttable[i].mem_addr;
					rpttable[i].mem_addr = stat.mem_addr;
				}else if(rpttable[i].diff2 == 99999){
						rpttable[i].diff2 = stat.mem_addr - rpttable[i].mem_addr;
						rpttable[i].mem_addr = stat.mem_addr;
				}else if(rpttable[i].diff2 != rpttable[i].diff && rpttable[i].diff != 99999){
						rpttable[i].diff = rpttable[i].diff2;
						rpttable[i].diff2 = 99999;
						rpttable[i].mem_addr = stat.mem_addr;
				}
				if(rpttable[i].diff != 99999){
					int j = 0;
					int diff_least_used = 0;
					int time_count = 9999999;
					bool mu_found = false;
					for(j = 0; j < 3; j++){
						if(rpttable[i].stats[j].diff == rpttable[i].diff){
							rpttable[i].stats[j].times_used++;
							mu_found = true;
						}
						if(rpttable[i].stats[j].times_used < time_count){
							time_count = rpttable[i].stats[j].times_used;
							diff_least_used = j;
						}
					}
					if(!mu_found){
						rpttable[i].stats[diff_least_used].diff = rpttable[i].diff;
						rpttable[i].stats[diff_least_used].times_used = 1;
					}
				}
				if(rpttable[i].diff2 == rpttable[i].diff && rpttable[i].diff != 99999){
					fetch = stat.mem_addr+rpttable[i].diff;
				}
			}
		}
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
			rpt.diff2 = 99999;
			i = 0;
			for(i = 0; i < 3; i++){
				MU m;
				m.diff = 0;
				m.times_used = 0;
				rpt.stats[i] = m;
			}
			rpttable[length] = rpt;
		}
		else if(fetch != 0 && MAX_PHYS_MEM_ADDR > fetch){
			if (!in_cache(fetch)) {

				issue_prefetch(fetch);
			}
		}else if(found){
			i = 0;
			int possible_diff = 0;
			int times_used = 0;
			for(i = 0; i < 3; i++){
				if(rpttable[found_index].stats[i].times_used > times_used){
					times_used = rpttable[found_index].stats[i].times_used;
					possible_diff = rpttable[found_index].stats[i].diff;
				}
			}
			fetch = stat.mem_addr + possible_diff;
			if(!in_cache(fetch) && MAX_PHYS_MEM_ADDR > fetch){
				issue_prefetch(fetch);
			}
		}
		
		/*else
		{
			int next = 50;
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

