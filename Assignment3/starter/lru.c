#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

// Timestamp 
int time;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	// Find the page with smallest timestamp
	int i, minTime = time, minPos;
	for(i = 0; i < memsize; ++i) {
		if(coremap[i].timestamp < minTime) {
			minTime = coremap[i].timestamp;
			minPos = i;
		}
	}
	
	return minPos;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	// On reference, refresh corresponding physical memory's timestamp
	coremap[p -> frame >> PAGE_SHIFT].timestamp = time++;
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	int i;
	time = 0;	// Init time

	// Initialize all timestamp to 0 for each page in physical memory
	for(i = 0; i < memsize; ++i) {
		coremap[i].timestamp = 0;
	}
}
