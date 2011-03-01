/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
struct RPT {
    Addr pc;        
		Addr last_mem_addr; 
		int* deltas;
		int deltai;
		int fill;
		Addr last_prefetch;
};
const int l = 100;
const int deltal = 10;
RPT rpttable[l];

Addr* DC(int i);
void PF(int i, Addr* cand);

int length = -1;
int far = 0;
int n_cand = 0;
bool begindelete = false;
void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
		bool found = false;
		int foundi = 0;
		int i = 0;
		for(i = 0; i < far; i++){
			if(rpttable[i].pc == stat.pc){
				found = true;
				foundi = i;
				if(stat.mem_addr - rpttable[i].last_mem_addr != 0){
					if(rpttable[i].deltai < deltal-1){
						rpttable[i].deltas[rpttable[i].deltai++] = stat.mem_addr - rpttable[i].last_mem_addr;
						if(rpttable[i].fill != deltal-1){
							rpttable[i].fill++;
						}
					}else{
						rpttable[i].deltas[rpttable[i].deltai] = stat.mem_addr - rpttable[i].last_mem_addr;
						rpttable[i].deltai = 0;
						rpttable[i].fill = deltal-1;
					}
					rpttable[i].last_mem_addr = stat.mem_addr;
				}
			}
		}
		if(!found){
			if(length < l-1){
				length++;
				if(far != l){
					far++;
				}
			}else{
				begindelete = true;	
				length = 0;
			}
			if(begindelete){
				int* deltas = rpttable[length].deltas;
				delete[] deltas;
			}
			RPT rpt;
			rpt.pc = stat.pc;
			rpt.deltai = 0;
			rpt.fill = 0;
			rpt.last_mem_addr = stat.mem_addr;
			rpt.last_prefetch = 0;
			rpt.deltas = new int[deltal];
			rpttable[length] = rpt;
		}else if(rpttable[foundi].fill > 1) {
			Addr* candidates = DC(foundi);
			PF(foundi, candidates);
			delete[] candidates;
		}else if(stat.mem_addr + (BLOCK_SIZE * 50) < MAX_PHYS_MEM_ADDR && !in_cache(stat.mem_addr + (BLOCK_SIZE * 50))){
			issue_prefetch(stat.mem_addr + (BLOCK_SIZE * 50));
		}
}
Addr* DC(int i){
	Addr* candidates = new Addr[deltal];
	n_cand = 0;
	int d1 = rpttable[i].deltas[rpttable[i].deltai];
	int d2 = 0;
	int delta = 0;
	if(rpttable[i].deltai == 0){
		delta = deltal-1;
	}else{
		delta = rpttable[i].deltai-1;
	}
	d2 = rpttable[i].deltas[delta];
	Addr addr = rpttable[i].last_mem_addr;
	int u = 0;
	for(u = 0; u < deltal; u++){
		int v = u + 1;
		if(u == deltal-1){
			v = 0;	
		}
		if(u <= rpttable[i].fill && v <= rpttable[i].fill && v != rpttable[i].deltai){
			if(rpttable[i].deltas[u] == d2 && rpttable[i].deltas[v] == d1){
				int w = v;
				while(w != u){
					if(w < rpttable[i].fill){
						w++;
					}else{
						w = 0;
					}
					if(w != u && w <= rpttable[i].fill){
						Addr ad = addr + rpttable[i].deltas[w];
						int x = 0;
						bool found = false;
						for(x = 0; x < n_cand; x++){
							if(candidates[x] == ad){
								found = true;
							}
						}
						if(!found && n_cand < deltal){
							candidates[n_cand++] = ad;
						}
					}
				}
			}
		}
	}
	return candidates;
}

void PF(int i, Addr* cand){
	int j = 0;
	for(j = 0; j < n_cand; j++){
		if(!in_cache(cand[j]) && !in_mshr_queue(cand[j]) && cand[j] <= MAX_PHYS_MEM_ADDR && cand[j] != rpttable[i].last_prefetch){
			issue_prefetch(cand[j]);
			rpttable[i].last_prefetch = cand[j];
		}else if(!in_cache(cand[j]+BLOCK_SIZE) && !in_mshr_queue(cand[j]+BLOCK_SIZE) && cand[j]+BLOCK_SIZE <= MAX_PHYS_MEM_ADDR && cand[j]+BLOCK_SIZE != rpttable[i].last_prefetch) {
			issue_prefetch(cand[j]+BLOCK_SIZE);
		}
	}
}
void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}

