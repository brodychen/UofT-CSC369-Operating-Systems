#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>

#include "ext2.h"

/**
 * This program works like mkdir, creating the final directory on the 
 * specified path on the disk.
 *
 * @arg1: An ext2 formatted virtual disk
 * @arg2: An absolute path on this disk
 *
 * @return: 	Success:					0
 * 				Path not exist: 			ENOENT
 * 				Directory already exists: 	EEXIST
 */

extern unsigned char *disk;				// Global pointer to mmap the disk to

extern struct ext2_super_block *sb;		// Pointer to super block
extern struct ext2_group_desc *gt;		// Pointer to group table
extern struct ext2_inode *ind_tbl;		// Pointer to inode table
extern char *blk_bmp;					// Pointer to block bitmap
extern char *ind_bmp;					// Pointer to inode bitmap


int main(int argc, char **argv) {

	if(argc != 3) {
		fprintf(stderr, "Usage: %s <image file name> <directory path>\n", argv[0]);
		exit(1);
	}
	int fd = open(argv[1], O_RDWR);		// Allow r & w

	// Map disk to memory, allow r & w
	// Share this mapping between processes
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	// Pointer to super block (number 2)
	sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	// Pointer to group table (number 3). Only 1 group in assignment, so take first one
	gt = (struct ext2_group_desc *)(disk + EXT2_BLOCK_SIZE * 2);
	// Pointer to inode table (number 6+)
	ind_tbl = (struct ext2_inode *)(disk + EXT2_BLOCK_SIZE * gt[0].bg_inode_table);
	// Pointer to block bitmap (number 4)
	blk_bmp = disk + EXT2_BLOCK_SIZE * gt[0].bg_block_bitmap;
	// Pointer to inode bitmap (number 5)
	ind_bmp = disk + EXT2_BLOCK_SIZE * gt[0].bg_inode_bitmap;

	
	// Cd to directory where new dir will be created
	// Partition new dir and sub-directories
	int i = strlen(argv[2]) - 1;
	if(argv[2][i] == '/') argv[2][i--] = '\0';		// Eliminate trailing slashes
	while(i >= 0) {									// Move i to last '/'
		if(argv[2][i] != '/') --i;
		else break;
	}
	if(argv[2][0] == '.' && argv[2][1] == '/') {	// Eliminate './' at beginning
		argv[2] += 2;
		i -= 2;
	}

	int parent_dir_inode = EXT2_ROOT_INO;			// Init with root inode
	// New sub-dir not in root, first cd to working directory
	if(i != -1) {
		parent_dir_inode = cd(argv[2], i);

		// Cd fails because path not exist
		if(parent_dir_inode == -ENOENT) {
			fprintf(stderr, "Path not exists\n");
			return ENOENT;
		}
	}

	// Check if same directory (but not types) already exists within parent directory
	++i;
	struct ext2_dir_entry *possible_dup_dir_ent 
		= search_in_dir_inode(argv[2] + i, strlen(argv[2]) - i, ind_tbl + parent_dir_inode - 1);
	if(possible_dup_dir_ent != NULL && (get_inode_mode(possible_dup_dir_ent -> inode) & EXT2_S_IFDIR)) {
		fprintf(stderr, "Directory already exists\n");
		exit(EEXIST);
	}

	// Determine the length of new directory entry
	int new_ent_name_len = strlen(argv[2]) - i;
	int new_ent_rec_len = 8 + new_ent_name_len;
	// Pad new directory entry length to align with 4
	new_ent_rec_len += 3; new_ent_rec_len >>= 2; new_ent_rec_len <<= 2;

	// Find the next block in memory for new directory
	struct ext2_dir_entry *new_ent = search_in_dir_inode(NULL, new_ent_rec_len, ind_tbl + parent_dir_inode - 1);

	// Setup context for this new dir
	// new_ent -> inode = 0;									// Tobe set later
	new_ent -> rec_len = 1024 - ((unsigned char *)new_ent - disk) % 1024;
	new_ent -> name_len = new_ent_name_len;
	new_ent -> file_type = EXT2_FT_DIR;
	memcpy((unsigned char *)new_ent + 8, argv[2] + i, new_ent_name_len);

	// Find a empty inode for this directory
	int new_inode = allocate_inode();
	new_ent -> inode = new_inode;
	struct ext2_inode *new_inode_p = ind_tbl + new_inode - 1;
	new_inode_p -> i_mode = EXT2_S_IFDIR;	// Set new inode mode to directory

	// Allocate a new block for this directory's inode
	int new_block = allocate_block() - 1;
	(new_inode_p -> i_block)[0] = new_block;
	// Setup self ent
	struct ext2_dir_entry *self_ent = (struct ext2_dir_entry *)(disk + EXT2_BLOCK_SIZE * new_block);
	self_ent -> inode = new_inode;
	self_ent -> rec_len = 12;
	self_ent -> name_len = 1;
	self_ent -> file_type = EXT2_FT_DIR;
	memset(self_ent -> name, '.', 1);
	// Setup parent ent
	struct ext2_dir_entry *parent_ent = (struct ext2_dir_entry *)(disk + EXT2_BLOCK_SIZE * new_block + 12);
	parent_ent -> inode = parent_dir_inode;
	parent_ent -> rec_len = 1012;
	parent_ent -> name_len = 2;
	parent_ent -> file_type = EXT2_FT_DIR;
	memset(parent_ent -> name, '.', 2);

	// Update block-group-descriptor
	gt -> bg_used_dirs_count -= 1;
	
	return 0;
}