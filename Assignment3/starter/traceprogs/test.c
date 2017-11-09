#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define ROUND 10000

typedef struct _Block {
	char data[PAGE_SIZE];
} Block;

int main() {
	int i;
	Block **heap = malloc(ROUND * sizeof(Block *));
	for(i = 0; i < ROUND; ++i) {
		heap[i] = malloc(Block);
		Block a;
	}
	for(int i = 0; i < ROUND; ++i) {
		free(heap[i]);
	}
	return;
}