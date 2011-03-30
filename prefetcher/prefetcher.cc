/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
*/

#include "interface.hh"

#include "nval.h"

struct RPT {
	Addr pc;        
	Addr last_mem_addr; 
	short* deltas;
	unsigned char deltai;
	unsigned char fill;
	Addr last_prefetch;
};
const int l = 120;
const int deltal = 16;
RPT rpttable[l];

Addr cachetable[MAX_QUEUE_SIZE];
int cachefill = 0;

bool in_cache_queue(Addr a){
	Addr block = a/BLOCK_SIZE;
	int i = 0;
	for(i = 0; i < cachefill; i++){
		if(block == cachetable[i]){
			return true;
		}
	}
	return false;
}

void insert_cache_queue(Addr a){
		Addr block = a/BLOCK_SIZE;
		if(cachefill != 0){
			int i = 0;
			for(i = cachefill-1; i > 0; i--){
				cachetable[i] = cachetable[i-1];
			}
		}
		cachetable[0] = block; 
		if(cachefill != MAX_QUEUE_SIZE){
			cachefill++;
		}

}

void remove_from_cache_queue(Addr a){
	Addr block = a/BLOCK_SIZE;
	int i = 0;
	for(i = 0; i < cachefill; i++){
		if(cachetable[i] == block){
			int j = 0;
			for(j = i+1; j < cachefill; j++){
				cachetable[i] = cachetable[j];
			}
			cachefill--;
			break;
		}
	}
}

bool canPrefetch(Addr a){
	return (a <= MAX_PHYS_MEM_ADDR/* && !get_prefetch_bit(a)*/ && !in_mshr_queue(a) && !in_cache(a) && !in_cache_queue(a));
}

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
	begindelete = false;
	n_cand = 0;
	int i = 0;
	for(i = 0; i < far; i++){
		if(rpttable[i].pc == stat.pc){
			found = true;
			foundi = i;
			if(stat.mem_addr - rpttable[i].last_mem_addr != 0 ){
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
			}
			if(((stat.mem_addr - rpttable[i].last_mem_addr) > 32768)){//Fucker seg med det her|| ((stat.mem_addr - rpttable[i].last_mem_addr) < (-65536))){
				rpttable[i].deltai = 0;
				rpttable[i].fill = 0;
			}
			rpttable[i].last_mem_addr = stat.mem_addr;
		}
	}
	if(!found){
		if(length < l-1){																																																																																																																length++;
			if(far != l){
				far++;
			}
		}else{
			begindelete = true;	
			length = 0;
		}
		if(begindelete){
			short* deltas = rpttable[length].deltas;
			delete[] deltas;
		}
		RPT rpt;
		rpt.pc = stat.pc;
		rpt.deltai = 0;
		rpt.fill = 0;
		rpt.last_mem_addr = stat.mem_addr;
		rpt.last_prefetch = 0;
		rpt.deltas = new short[deltal];
		rpttable[length] = rpt;
	}else if(rpttable[foundi].fill > 2) {
		Addr* candidates = DC(foundi);
		PF(foundi, candidates);
		delete[] candidates;
	}
        // Sequential failover.
	if(n_cand == 0){
                Addr pref = stat.mem_addr;
                int n;
                for (n = 0; n < N_VAL; n++) {
                  pref += BLOCK_SIZE;
                  if(canPrefetch(pref)){
			issue_prefetch(pref);
			insert_cache_queue(pref);
//			set_prefetch_bit(pref);
                  }
                }
	}
}


Addr* DC(int i){
	Addr* candidates = new Addr[deltal];
	int patterns[deltal];
	int n_patterns = 0;
	n_cand = 0;
	short d1 = rpttable[i].deltas[rpttable[i].deltai];
	short d2 = 0;
	int delta = 0;
	if(rpttable[i].deltai == 0){
		delta = deltal-1;
	}else{
		delta = rpttable[i].deltai-1;
	}
	d2 = rpttable[i].deltas[delta];
	int u = delta-1;
	if(rpttable[i].deltai == 0){
		u = deltal-2;
	}else if(rpttable[i].deltai == 1){
		u = deltal-1;
	}else{
		u = rpttable[i].deltai-2;
	}
	while(u != delta && u <= rpttable[i].fill){
		int v = u + 1;
		if(u == rpttable[i].fill){
			v = 0;	
		}
		if(u <= rpttable[i].fill && v <= rpttable[i].fill && v != rpttable[i].deltai){
			if(rpttable[i].deltas[u] == d2 && rpttable[i].deltas[v] == d1){
				patterns[n_patterns++] = (v < rpttable[i].fill)?v+1:0;
			}
		}
		if(u == 0){
			u = rpttable[i].fill;
		}else{
			u--;
		}
	}
	int j = 0;
	//for(j = 0; j < n_patterns; j++){
		Addr addr = rpttable[i].last_mem_addr;
		int w = patterns[j];
		int next = (j+1 < n_patterns)?patterns[j+1]:patterns[0];
		bool stop = false;
		while(w != next && !stop){
			
			if(w <= rpttable[i].fill){
				addr = addr + rpttable[i].deltas[w];
				if(addr > 0){
					int x = 0;
					bool found = false;
					for(x = 0; x < n_cand; x++){
						if(candidates[x] == addr || candidates[x]/BLOCK_SIZE == addr/BLOCK_SIZE){
							found = true;
						}
					}
					if(!found && n_cand < deltal){
						candidates[n_cand++] = addr;
					}
				}else{
					//Gaar negativ, stop this shit
					stop = true;
				}
			}
			if(w < rpttable[i].fill){
				w++;
			}else{
				w = 0;
			}
		}
	//}
	return candidates;
}


void PF(int i, Addr* cand){

	int j = 0;
	for(j = 0; j < n_cand; j++){
		if(canPrefetch(cand[j]) && cand[j] != rpttable[i].last_prefetch){
			issue_prefetch(cand[j]);
			rpttable[i].last_prefetch = cand[j];
			insert_cache_queue(cand[j]);
			//set_prefetch_bit(cand[j]);
		}
	}
}

void prefetch_complete(Addr addr) {
//	clear_prefetch_bit(addr);
	remove_from_cache_queue(addr);
	/*
	 * Called when a block requested by the prefetcher has been loaded.
	 */
}


