#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

#include "sim.h"


// extern int memsize;
extern unsigned memsize;

extern int debug;

extern struct frame *coremap;

int traceSize;		// Total num of memory accesses
int *vPageRecord;	// List of virtual page accesses 
int *nextAccessTime;	// Time of next access of same virtual page
int time;

typedef struct _Record {
	int vPage;
	int time;
	struct _Record *left;
	struct _Record *right;
} Record;

// Function declarations
int update(Record *root, int vPage, int timeNew);
void destroy_tree(Record *root);

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	int i;
	int latestTime = 0, latestFrame;

	// Search for a frame with latest next access time
	for(i = 0; i < memsize; ++i) {

		// Next access time is -1, this frame never accessed again
		if(coremap[i].nextAccessTime == -1) {
			return i;
		} 

		// Keep searching
		else if(coremap[i].nextAccessTime > latestTime) {
			latestTime = coremap[i].nextAccessTime;
			latestFrame = i;
		}
		
	}

	return latestFrame;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	
	// Update nextAccessTime in coremap
	coremap[p -> frame >> PAGE_SHIFT].nextAccessTime = nextAccessTime[time];

	// Clean data structures on last reference
	if(++time == traceSize) {
		free(vPageRecord);
		free(nextAccessTime);
	}
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	int i;
	char type;			// Placeholder
	addr_t vaddr;	
	char buf[MAXLINE];	// Buffer for reading file
	Record *root;		// Binary tree
	FILE *tfp = stdin;

	// Load tracefile
	if(tracefile != NULL) {
		if((tfp = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
		}
	}

	// Count line numbers of the tracefile, malloc arrays
	traceSize = 0;
	while(fgets(buf, MAXLINE, tfp) != NULL) {
		if(buf[0] == '=') continue;
		++traceSize;
	}
	rewind(tfp);
	vPageRecord = calloc(1, traceSize * sizeof(int));
	nextAccessTime = calloc(1, traceSize * sizeof(int));

	// Parse virtual page into vPageRecord
	for(i = 0; i < traceSize; ++i) {
		fgets(buf, MAXLINE, tfp);
		if(buf[0] == '='){
			--i;
			continue;
		}
		sscanf(buf, "%c %lx", &type, &vaddr);
		vPageRecord[i] = vaddr >> PAGE_SHIFT;
	}

	// Build BST to construct array nextAccessTime
	root = calloc(1, sizeof(Record));
	root -> vPage = vPageRecord[traceSize - 1];
	root -> time = traceSize - 1;
	for(i = traceSize - 2; i >= 0; --i)  {
		nextAccessTime[i] = update(root, vPageRecord[i], i);
	}
	destroy_tree(root);

	// Set initial time to 0, incremented on reference
	time = 0;
}


/**
 * BST helper function for constructing array nextAccessTime
 * Called upon every memory reference, combination of search and insert
 * If vPage exists, return its previous access time
 * If vPage doesn't exist, return -1
 * Update vPage's time to timeNew in both cases
 */
int update(Record *root, int vPage, int timeNew) {

	assert(root);
	// Found
	if(root -> vPage == vPage) {
		int timeOld = root -> time;
		root -> time = timeNew;
		return timeOld;
	}

	else if(vPage < root -> vPage) {
		// If null, insert new and return 1
		if(root -> left == NULL) {
			root -> left = calloc(1, sizeof(Record));
			root -> left -> vPage = vPage;
			root -> time = timeNew;
			return -1;
		}
		// Found
		else return update(root -> left, vPage, timeNew);
	}

	else {
		// If null, insert new and return 1
		if(root -> right == NULL) {
			root -> right = calloc(1, sizeof(Record));
			root -> right -> vPage = vPage;
			root -> time = timeNew;
			return -1;
		}
		// Found
		else return update(root -> right, vPage, timeNew);
	}

}

void destroy_tree(Record *root) {
	if(root == NULL) return;
	if(root -> left) destroy_tree(root -> left);
	if(root -> right) destroy_tree(root -> right);
	free(root);
	root = NULL;
}

