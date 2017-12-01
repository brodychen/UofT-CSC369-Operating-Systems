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
 * Restores file.
 *
 * @arg1: An ext2 formatted virtual disk
 * @arg2: Absolute path to link of file
 *
 * @return:	Success:	0
 * 			Not fully restored:		ENOENT
 */

extern unsigned char *disk;				// Global pointer to mmap the disk to

extern struct ext2_super_block *sb;		// Pointer to super block
extern struct ext2_group_desc *gt;		// Pointer to group table
extern struct ext2_inode *ind_tbl;		// Pointer to inode table
extern char *blk_bmp;					// Pointer to block bitmap
extern char *ind_bmp;					// Pointer to inode bitmap

int main() {
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <image file name> <file path>\n", argv[0]);
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

	int parent_dir_inode = EXT2_ROOT_INO - 1;			// Init with root inode
	// New sub-dir not in root, first cd to working directory
	if(i != -1) {
		parent_dir_inode = cd(argv[2], i);

		// Cd fails because path not exist
		if(parent_dir_inode == -ENOENT) {
			fprintf(stderr, "Path not exists\n");
			exit(ENOENT);
		}
	}
	
	// Check if file (but not directory) with same name already exists
	++i;
	struct ext2_dir_entry *possible_dup_dir_ent 
		= search_in_dir_inode(argv[2] + i, strlen(argv[2]) - i, ind_tbl + parent_dir_inode);
	if(possible_dup_dir_ent == NULL) {
		fprintf(stderr, "Directory not exists\n");
		return EEXIST;
    }
    else if(get_inode_mode(possible_dup_dir_ent -> inode) & EXT2_S_IFDIR) {
        fprintf(stderr, "Can't remove directory\n");
        return EISDIR;
    }

}

/**
 * Restore inode inode. 
 * @arg1: Inode number, starting at 1
 * @return:	Success:	1
 * 			Fail:		0
 */
int restore_inode(int inode) {
	--inode;
	
	// If inode already taken, file has been overwritten
	if((ind_bmp[inode >> 3] & (1 << (inode % 8))) == 0) {
		return 0;
	}
	
	ind_bmp[inode >> 3] |= (1 << (inode % 8));
	return 1;
}

/**
 * Recursively restore an dir/file
 * 
 * @arg1: inode number, starting from 1
 * @arg2: whether recursive
 * 
 * @return: 	Success:	1
 * 				Fail:		0
 */



int restore_inode(int inode, bool recursive) {
	--inode;

	int i;
	struct ext2_inode *inode_p = ind_tbl + inode;

	for(i = 0; i < 12; ++i) {
		

	}
}


int restore_block() {

}

int restore_dir_block() {
	
}