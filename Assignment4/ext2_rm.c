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
 * This program works like rm.
 * 
 * @arg1: An ext2 formatted virtual disk
 * (@arg2: -r flag)
 * @arg3: Absolute path to file/link
 *
 * @return: 	Success: 	0
 *              No -r flag but dir:  EISDIR
 *              File/path not found: EEXIST
 */


// Forward declarations
void free_block(int block);
void free_dir_block(int block);
void free_inode(int inode, bool recursive);


extern unsigned char *disk;			    // Global pointer to mmap the disk to

extern struct ext2_super_block *sb;	    // Pointer to super block
extern struct ext2_group_desc *gt;		// Pointer to group table
extern struct ext2_inode *ind_tbl;		// Pointer to inode table
extern char *blk_bmp;					// Pointer to block bitmap
extern char *ind_bmp;					// Pointer to inode bitmap

// Used to store previous dir entry of the entry to be removed
extern struct ext2_dir_entry *prev_dir_entry;

int main(int argc, char **argv) {

    // Parse input
    bool recursive = 0;     // Whether recursion
    bool input_valid = 1;   // Whether input is valid
    // if(argc == 4) {
    //     if(argv[2][0] == '-' && argv[2][1] == 'r') {
    //         recursive == 1;
    //         argv[2] = argv[3];  // argv[2] points to file/dir name
    //     }
    //     else {
    //         input_valid = 0;
    //     }
    // }
    if(argc != 3) {
        input_valid = 0;
    } 
    if(!input_valid) {
        // fprintf(stderr, "Usage: %s <image file name> <flag> <path>\n");
        fprintf(stderr, "Usage: %s <image file name> <path>\n", argv[0]);
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
    
    // Check if file to remove not exists or is a directory
	++i;
	struct ext2_dir_entry *possible_dup_dir_ent 
		= search_in_dir_inode(argv[2] + i, strlen(argv[2]) - i, ind_tbl + parent_dir_inode);
	if(possible_dup_dir_ent == NULL) {
		fprintf(stderr, "File not exists\n");
		return EEXIST;
    }
    else if(get_inode_mode(possible_dup_dir_ent -> inode) & EXT2_S_IFDIR) {
        fprintf(stderr, "Can't remove directory\n");
        return EISDIR;
    }

    // Remove this directory entry
    free_inode(possible_dup_dir_ent -> inode, false);

    // Set context of previous dir entry (if exists)
    if(prev_dir_entry){
        prev_dir_entry -> rec_len += possible_dup_dir_ent -> rec_len;
    }
    // Removed entry is first in dir block
    else {
        // Set first inode to 0 (unrecoverable)
        possible_dup_dir_ent -> inode = 0;
    }

}

/**
 * This function frees block with index block, by simply clearing block from bitmap.
 * @arg1: block number (starting from 0)
 */
void free_block(int block) {
    --block;
    assert((blk_bmp[block >> 3] & (1 << (block % 8))) >= 1);    // Make sure bit is set
    blk_bmp[block >> 3] &= (~(1 << (block % 8)));               // Clear bitmap
    ++block;
}

/**
 * This function frees a directory block, and clears block from bitmap
 * Double recursion between free_inode() and free_dir_block()
 * @arg1: block number (starting from 0)
 */
void free_dir_block(int block) {
    --block;
    assert((blk_bmp[block >> 3] & (1 << (block % 8))) >= 1);    // Make sure bit is set
    blk_bmp[block >> 3] &= (~(1 << (block % 8)));               // Clear bitmap
    ++block;
    sb -> s_free_blocks_count += 1;
    gt -> bg_free_blocks_count += 1;
    gt -> bg_used_dirs_count += 1;
    
    unsigned char *block_p = disk + block * EXT2_BLOCK_SIZE;		// Start of block
	unsigned char *cur = block_p;									// Search pos

    // Get current directory size, because different with rec_len for last dir block
    int cur_dir_size, prev_dir_size;
    while(1) {

        prev_dir_size = cur_dir_size;
		cur_dir_size = 8 + ((struct ext2_dir_entry *)(cur)) -> name_len;
        cur_dir_size += 3; cur_dir_size >>= 2; cur_dir_size <<= 2;
        
        // Free this inode recursively
        free_inode(((struct ext2_dir_entry *)(cur)) -> inode, true);
        
        // Update cur to position of next file in this block
		// cur += ((struct ext2_dir_entry *)cur) -> rec_len;
		cur += ((struct ext2_dir_entry *)(cur)) -> rec_len;
        
        // Not found if p reach end of block
        if(cur - block_p >= EXT2_BLOCK_SIZE) return;
    }

}

/**
 * This function frees every block belonging to this inode.
 * Clears inode bitmap.
 * Double recursion between free_inode() and free_dir_block()
 * 
 * @arg1: inode number (starting from 1)
 * @arg2: Whether rm recursively
 */
void free_inode(int inode, bool recursive) {

    --inode;
    // Check inode file type
    if((ind_tbl[inode].i_mode) & EXT2_S_IFDIR && !recursive) {
        fprintf(stderr, "Cannot remove directory\n");
        exit(EISDIR);
    }

    // Clear bit map
    assert((ind_bmp[inode >> 3] & (1 << (inode % 8))) >= 1);    // Make sure set
    ind_bmp[inode >> 3] &= (~(1 << (inode % 8)));
    sb -> s_free_inodes_count += 1;
    gt -> bg_free_inodes_count += 1;

    // Remove every block in i_block
    int i, j, k;
    struct ext2_inode *inode_p = ind_tbl + inode;
    for(i = 0; i < 12; ++i) {   // Direct blocks

        // Value 0 suggests no further block defined, return
		if((inode_p -> i_block)[i] == 0) {
            return;
        }

        if(!recursive || ((inode_p -> i_mode & EXT2_S_IFDIR) == 0)) {
            free_block((inode_p -> i_block)[i]);
        }
        else {
            free_dir_block((inode_p -> i_block)[i]);
        }
        // (inode_p -> i_block)[i] = 0;
    }

    // File also in indirect blocks, keep removing 
    int *indirect_block = (int *)(disk + (inode_p -> i_block)[13] * EXT2_BLOCK_SIZE);

    for(i = 0; i < 256; ++i) {

        // Value 0 suggests no further block defined, return
        if(indirect_block[i] == 0) {
            return;
        }
        
        if(!recursive || ((inode_p -> i_mode & EXT2_S_IFDIR) == 0)) {
            free_block(indirect_block[i]);
        }
        else {
            free_dir_block(indirect_block[i]);
        }
        // indirect_block[i] = 0;
    }

    // File also in double indirect blocks, keep removing
    indirect_block = (int *)(disk + (inode_p -> i_block)[14] * EXT2_BLOCK_SIZE);

    for(i = 0; i < 256; ++i) {
        int *db_indirect_block = (int *)(disk + indirect_block[i] * EXT2_BLOCK_SIZE);
        // Clear bit map
        assert((ind_bmp[inode >> 3] & (1 << (inode % 8))) >= 1);    // Make sure set
        ind_bmp[inode >> 3] &= (~(1 << (inode % 8)));
        sb -> s_free_inodes_count += 1;
        gt -> bg_free_inodes_count += 1;
        for(j = 0; j < 256; ++j) {

            // Value 0 suggests no further block defined, return
            if(db_indirect_block[j] == 0) {
                return;
            }

            if(!recursive || ((inode_p -> i_mode & EXT2_S_IFDIR) == 0)) {
                free_block(db_indirect_block[j]);
            }
            else {
                free_dir_block(db_indirect_block[j]);
            }
            // db_indirect_block[j] = 0;
        }
    }

    // File also in triple indirect blocks, keep removing
    indirect_block = (int *)(disk + (inode_p -> i_block)[14] * EXT2_BLOCK_SIZE);

    for(i = 0; i < 256; ++i) {
        int *db_indirect_block = (int *)(disk + indirect_block[i] * EXT2_BLOCK_SIZE);

        for(j = 0; j < 256; ++j) {
            int *tp_indirect_block = (int *)(disk + db_indirect_block[j] * EXT2_BLOCK_SIZE);

            for(k = 0; k < 256; ++k) {

                // Value 0 suggests no further block defined, i.e. not found
                if(tp_indirect_block[k] == 0) {
                    return;
                }	

                if(!recursive || ((inode_p -> i_mode & EXT2_S_IFDIR) == 0)) {
                    free_block(db_indirect_block[j]);
                }
                else {
                    free_dir_block(db_indirect_block[j]);
                }
                // db_indirect_block[j] = 0;
            }
        }
    }
}