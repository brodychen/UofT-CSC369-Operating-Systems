#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include "ext2.h"

unsigned char *disk;

void print_inode(struct ext2_inode *inode_table, unsigned int idx);

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <image file name>\n", argv[0]);
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    printf("Inodes: %d\n", sb->s_inodes_count);
    printf("Blocks: %d\n", sb->s_blocks_count);
    
    // Locate the block group table
    // Because only 1 group in this assignment, take the first one
    struct ext2_group_desc *gt = (struct ext2_group_desc *)(disk + 2048);
    printf("Block group:\n");
    printf("    block bitmap: %d\n", gt[0].bg_block_bitmap);
    printf("    inode bitmap: %d\n", gt[0].bg_inode_bitmap);
    printf("    inode table: %d\n", gt[0].bg_inode_table);
    printf("    free blocks: %d\n", gt[0].bg_free_blocks_count);
    printf("    free inodes: %d\n", gt[0].bg_free_inodes_count);
    printf("    used_dirs: %d\n", gt[0].bg_used_dirs_count);

    // Print block bitmap
    unsigned int i;
    printf("Block bitmap: ");
    char *block_bit = (char *)disk + gt[0].bg_block_bitmap * 0x400;
    for(; block_bit < (char *)disk + gt[0].bg_block_bitmap * 0x400 + ((sb -> s_blocks_count) >> 3); ++block_bit) {
        for(i = 1; i <= 0x80; i <<= 1) {
            printf("%d", ((*block_bit) & i) > 0);
        }
        printf(" ");
    }
    printf("\n");

    // Print inode bitmap
    printf("Inode bitmap: ");
    char *inode_bit = (char *)disk + gt[0].bg_inode_bitmap * 0x400;
    for(; inode_bit < (char *)disk + gt[0].bg_inode_bitmap * 0x400 + ((sb -> s_inodes_count) >> 3); ++inode_bit) {
        for(i = 1; i <= 0x80; i <<= 1) {
            printf("%d", ((*inode_bit) & i) > 0);
        }
        printf(" ");
    }
    printf("\n");

    // Print inodes
    printf("\nInodes:\n");
    // Print the second inode
    print_inode((struct ext2_inode *)((char *)disk + gt[0].bg_inode_table * 0x400), 2);
    // Print all other unreserved inodes
    inode_bit = (char *)disk + gt[0].bg_inode_bitmap * 0x400;
    unsigned int inode_idx = 1;
    for(; inode_bit < (char *)disk + gt[0].bg_inode_bitmap * 0x400 + ((sb -> s_inodes_count) >> 3); ++inode_bit) {
        for(i = 1; i <= 0x80; i <<= 1) {
            if(((*inode_bit) & i) && inode_idx >= 12) {  // Print only if this inode is set in bitmap
                print_inode((struct ext2_inode *)((char *)disk + gt[0].bg_inode_table * 0x400), inode_idx);
            }
            ++inode_idx;
        }
    }

    // Print directory blocks
    printf("\nDirectory Blocks:\n");

    
    return 0;
}

// Print the idx inode from inode table, idx starts from 1
void print_inode(struct ext2_inode *inode_table, unsigned int idx) {
    struct ext2_inode *p = inode_table + idx - 1;   // Pointer to this inode

    // Determine file type
    char type;
    if(p -> i_mode & EXT2_S_IFDIR) {    // Directory
        type = 'd';
    } else if(p -> i_mode & EXT2_S_IFREG) { // Regular file
        type = 'f';
    } else {
        type = '0';
        assert(0);
    }
    printf("[%d] type: %c size: %d links: %d blocks: %d\n", 
        idx, type, p -> i_size, p -> i_links_count, p -> i_blocks);

    // Print the blocks except the reserved blocks
    printf("[%d] Blocks: ", idx);
    int i;
    for(i = 0; i < 15; ++i) {
        // Value 0 in array suggests no further block being defined, break
        if((p -> i_block)[i] == 0) break;
        printf(" %d", (p -> i_block)[i]);
    }
    printf("\n");
}

