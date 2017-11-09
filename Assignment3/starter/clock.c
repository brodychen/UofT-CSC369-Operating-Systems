#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int arm;	// Position of "arm"

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	// Arm sweep through the pages
	while(1) {

		// If arm reached end, start another round
		if(arm >= memsize) arm = 0;

		// If reference bit is set, give a second chance
		if(coremap[arm].referenced == 1) {
			coremap[arm].referenced = 0;
		} else {
			return arm;
		}

		++arm;
	}
	return 0;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	// Set reference bit
	coremap[p -> frame >> PAGE_SHIFT].referenced = 1;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	int i;
	arm = 0;	// Init arm to beginning of physical memory

	// Initialize all timestamp to 0 for each page in physical memory
	for(i = 0; i < memsize; ++i) {
		coremap[i].referenced = 0;
	}
}
