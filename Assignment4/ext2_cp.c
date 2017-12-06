#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "ext2_helper.h"

/**
 * This program works like cp, copying a file from native OS to disk image.
 *
 * @arg1: An ext2 formatted virtual disk image
 * @arg2: A path on native OS
 * @arg3: An absolute path on virtual disk image
 *
 * @return: 	Success: 	0
 */

extern unsigned char *disk;			// Global pointer to mmap the disk to
 
extern struct ext2_super_block *sb;	// Pointer to super block
extern struct ext2_group_desc *gt;		// Pointer to group table
extern struct ext2_inode *ind_tbl;		// Pointer to inode table
extern char *blk_bmp;					// Pointer to block bitmap
extern char *ind_bmp;					// Pointer to inode bitmap

int main(int argc, char **argv) {

	if(argc != 4) {
		fprintf(stderr, "Usage: %s <image file name> <path on OS> <path on image>\n", argv[0]);
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

	// Open file on native OS
	FILE *fp;
	if((fp = fopen(argv[2], "r")) == NULL) {
		fprintf(stderr, "Can't open input file %s\n", argv[2]);
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

	// Cd to directory where new file will be created
	// Partition new dir and sub-directories
	int i = strlen(argv[3]) - 1;
	if(argv[3][i] == '/') argv[3][i--] = '\0';		// Eliminate trailing slashes
	if(argv[3][0] == '.' && argv[3][1] == '/') {	// Eliminate './' at beginning
		argv[3] += 2;
		i -= 2;
	}
	++i;

	int parent_dir_inode = EXT2_ROOT_INO - 1;		// Init with root inode
	// New sub-dir not in root, first cd to working directory
	if(argv[3][0] != '.') {
		parent_dir_inode = cd(argv[3], i);
		fprintf(stderr, "mkdir: cd to inode %d\n", parent_dir_inode + 1);

		// Cd fails because path not exist
		if(parent_dir_inode == -ENOENT) {
			fprintf(stderr, "cannot cp: No such file or directory\n");
			exit(ENOENT);
		}
	}

	// Extract name of source file
	int j = strlen(argv[2]) - 1;
	while(j >= 0) {
		if(argv[2][j] != '/') --j;
		else break;
	}
	++j;

	// Check if file (but not dir) with same name already exists within parent directory
	struct ext2_dir_entry *possible_dup_dir_ent 
		= search_in_dir_inode(argv[2] + j, strlen(argv[2]) - j, ind_tbl + parent_dir_inode);
	// if(possible_dup_dir_ent != NULL && (get_inode_mode(possible_dup_dir_ent -> inode) & EXT2_S_IFREG)) {
	if(possible_dup_dir_ent != NULL) {
 		fprintf(stderr, "File already exists, stop copying\n");
		exit(EEXIST);
	}

	// Determine the length of new file entry
	int new_ent_name_len = strlen(argv[2]) - j;
	int new_ent_rec_len = 8 + new_ent_name_len;
	// Pad new directory entry length to align with 4
	new_ent_rec_len += 3; new_ent_rec_len >>= 2; new_ent_rec_len <<= 2;

	// Find the next block in memory for new directory
	struct ext2_dir_entry *new_ent = search_in_dir_inode(NULL, new_ent_rec_len, ind_tbl + parent_dir_inode);

	// Setup context for this new dir
	new_ent -> rec_len = 1024 - ((unsigned char *)new_ent - disk) % 1024;
	new_ent -> name_len = new_ent_name_len;
	new_ent -> file_type = EXT2_FT_DIR;
	memcpy((unsigned char *)new_ent + 8, argv[2] + j, new_ent_name_len);
	
	// If inserted into a gap, restore new dir's rec_len
	if(((struct ext2_dir_entry *)((char *)new_ent + new_ent_rec_len)) -> inode >= 12 && 
		is_available_inode(((struct ext2_dir_entry *)((char *)new_ent + new_ent_rec_len)) -> inode) == 0) {
		new_ent -> rec_len = new_ent_rec_len;
	}

	// Modify rec_len for previous directory
	if(prev_dir_entry) {
		prev_dir_entry -> rec_len = prev_dir_entry -> name_len + 8;
		(prev_dir_entry -> rec_len) += 3;
		(prev_dir_entry -> rec_len) >>= 2;
		(prev_dir_entry -> rec_len) <<= 2;
		fprintf(stderr, "previous dir rec_len modified to %d\n", (prev_dir_entry -> rec_len));
	}

	// Find a empty inode for this file
	int new_inode = allocate_inode() - 1;
	new_ent -> inode = new_inode + 1;
	struct ext2_inode *new_inode_p = ind_tbl + new_inode;
	new_inode_p -> i_mode = EXT2_S_IFREG;	// Set new inode mode to regular file



	// Calculate number of blocks needed
	fseek(fp, 0, SEEK_END);
	int total_blocks = ftell(fp) / EXT2_BLOCK_SIZE + 1;
	rewind(fp);
	// fprintf(stderr, "Total blocks: %d\n", total_blocks);
	// fprintf(stderr, "Free blocks: %d\n", sb -> s_free_blocks_count);
	if(total_blocks > sb -> s_free_blocks_count) {		// Oversize file
		fprintf(stderr, "Oversize file\n");
		fclose(fp);
		exit(1);
	}


	// Allocate new blocks for the copied file

	int k;
	bool cp_complete = 0;

	for(i = 0; i < 12; ++i) {	// Direct blocks

		if(total_blocks <= 0) {
			cp_complete = 1;
			break;
		} 

		(new_inode_p -> i_block)[i] = allocate_block() - 1;
		if(sb -> s_free_blocks_count <= 0) {
			fprintf(stderr, "Space full\n");
			exit(1);
		}

		fread((void *)(disk + (new_inode_p -> i_block)[i] * EXT2_BLOCK_SIZE), EXT2_BLOCK_SIZE, 1, fp);
		--total_blocks;
	}

	if(cp_complete == 0) {		// Direct block not enough, use indirect blocks
		int *indirect_block = (int *)(disk + (new_inode_p -> i_block)[12] * EXT2_BLOCK_SIZE);
		*indirect_block = allocate_block() - 1;
		if(sb -> s_free_blocks_count <= 0) {
			fprintf(stderr, "Space full\n");
			exit(1);
		}

		for(i = 0; i < 256; ++i) {

			if(total_blocks <= 0) {
				cp_complete = 1;
				break;
			} 

			indirect_block[i] = allocate_block() - 1;
			if(sb -> s_free_blocks_count <= 0) {
				fprintf(stderr, "Space full\n");
				exit(1);
			}

			fread((void *)(disk + indirect_block[i] * EXT2_BLOCK_SIZE), EXT2_BLOCK_SIZE, 1, fp);
			--total_blocks;
		}
	}

	if(cp_complete == 0) {		// Indirect block not enough, use double indirect block
		int *indirect_block = (int *)(disk + (new_inode_p -> i_block)[13] * EXT2_BLOCK_SIZE);
		*indirect_block = allocate_block() - 1;
		if(sb -> s_free_blocks_count <= 0) {
			fprintf(stderr, "Space full\n");
			exit(1);
		}

		for(i = 0; i < 256; ++i) {
			int *db_indirect_block = (int *)(disk + indirect_block[i] * EXT2_BLOCK_SIZE);
			*db_indirect_block = allocate_block() - 1;
			if(sb -> s_free_blocks_count <= 0) {
				fprintf(stderr, "Space full\n");
				exit(1);
			}

			for(j = 0; j < 256; ++j) {

				if(total_blocks <= 0) {
					cp_complete = 1;
					break;
				} 

				db_indirect_block[j] = allocate_block() - 1;
				if(sb -> s_free_blocks_count <= 0) {
					fprintf(stderr, "Space full\n");
					exit(1);
				}

				fread((void *)(disk + db_indirect_block[j] * EXT2_BLOCK_SIZE), EXT2_BLOCK_SIZE, 1, fp);
				--total_blocks;
			}
		}
	}

	if(cp_complete == 0) {	// Double block not enough, use triple indirect block
		int *indirect_block = (int *)(disk + (new_inode_p -> i_block)[14] * EXT2_BLOCK_SIZE);\
		*indirect_block = allocate_block() - 1;
		if(sb -> s_free_blocks_count <= 0) {
			fprintf(stderr, "Space full\n");
			exit(1);
		}

		for(i = 0; i < 256; ++i) {
			int *db_indirect_block = (int *)(disk + indirect_block[i] * EXT2_BLOCK_SIZE);
			*db_indirect_block = allocate_block() - 1;
			if(sb -> s_free_blocks_count <= 0) {
				fprintf(stderr, "Space full\n");
				exit(1);
			}

			for(j = 0; j < 256; ++j) {
				int *tp_indirect_block = (int *)(disk + db_indirect_block[i] * EXT2_BLOCK_SIZE);
				*tp_indirect_block = allocate_block() - 1;
				if(sb -> s_free_blocks_count <= 0) {
					fprintf(stderr, "Space full\n");
					exit(1);
				}

				for(k = 0; k < 256; ++k) {

					if(total_blocks <= 0) {
						cp_complete = 1;
						break;
					} 
	
					tp_indirect_block[k] = allocate_block() - 1;
					if(sb -> s_free_blocks_count <= 0) {
						fprintf(stderr, "Space full\n");
						exit(1);
					}

					fread((void *)(disk + tp_indirect_block[k] * EXT2_BLOCK_SIZE), EXT2_BLOCK_SIZE, 1, fp);
					--total_blocks;
				}
			}
		}
	}

	if(cp_complete) {
		return 0;
	} 
	else {
		fprintf(stderr, "Copy failed, disk space exceeded.\n");
		exit(1);
	}
}

/**
 * This function copies data of size of one block, from src to block.
 * @arg1: Block number, starting from 0
 * @arg2: Pointer to 
 */
