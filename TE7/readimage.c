#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

unsigned char *disk;


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
    // printf("Superblock size: %d\n", sizeof(struct ext2_super_block));
    // printf("Inode size: %d\n", sizeof(struct ext2_inode));
    // printf("First data block: %d\n", sb -> s_first_data_block);
    // printf("Blocks per group: %d\n", sb -> s_blocks_per_group);
    // printf("Block group # of this superblock:%d\n", sb -> s_block_group_nr);

    // Locate the block group table
    // Because only 1 group in this assignment, take the first one
    struct ext2_group_desc *gt = (struct ext2_group_desc *)(disk + 2048);
    printf("Block group:\n");
    printf("\tblock bitmap: %d\n", gt[0].bg_block_bitmap);
    printf("\tinode bitmap: %d\n", gt[0].bg_inode_bitmap);
    printf("\tinode table: %d\n", gt[0].bg_inode_table);
    printf("\tfree blocks: %d\n", gt[0].bg_free_blocks_count);
    printf("\tfree inodes: %d\n", gt[0].bg_free_inodes_count);
    printf("\tused_dirs: %d\n", gt[0].bg_used_dirs_count);

    
    return 0;
}
