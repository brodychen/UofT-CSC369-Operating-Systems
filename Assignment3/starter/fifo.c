#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int time;	// Global time variable

/* Page to evict is chosen using the fifo algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
 

int fifo_evict() {
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
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {
	// Check if this page was just swapped in or allocated
	// If so, then update its timestamp, and clear new_swapin
	if(coremap[p -> frame >> PAGE_SHIFT].new_swapin) {
		coremap[p -> frame >> PAGE_SHIFT].new_swapin = 0;
		coremap[p -> frame >> PAGE_SHIFT].timestamp = time;
	}

	++time; 	// Increment time at each reference
	return;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	int i;
	time = 0;	// Init time

	// Initialize all timestamp to 0 for each page in physical memory
	for(i = 0; i < memsize; ++i) {
		coremap[i].new_swapin = 0;
		coremap[i].timestamp = 0;
	}
}
