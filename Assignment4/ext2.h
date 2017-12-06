/*
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/include/linux/minix_fs.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* MODIFIED by Bogdan Simion, Tian Ze Chen, and Karen Reid for CSC369
 * to remove unnecessary components for the assignment, clean up the code 
 * and fix some bugs */

#ifndef CSC369_EXT2_FS_H
#define CSC369_EXT2_FS_H

#include <errno.h>
#include <stdbool.h>

/* The ext2 block size used in the assignment. */
#define EXT2_BLOCK_SIZE 1024

/*
 * Structure of the super block
 */
struct ext2_super_block {
	unsigned int   s_inodes_count;      /* Inodes count */
	unsigned int   s_blocks_count;      /* Blocks count */
	/* Reserved blocks count is not used in the assignment. */
	unsigned int   s_r_blocks_count;    /* Reserved blocks count */
	unsigned int   s_free_blocks_count; /* Free blocks count */
	unsigned int   s_free_inodes_count; /* Free inodes count */
	
	/******************************************************************
	 * The rest of the ext2 superblock structure is irrelevent to the
	 * assignment, but is included below, for completeness.
	 ******************************************************************/

	unsigned int   s_first_data_block;  /* First Data Block */
	unsigned int   s_log_block_size;    /* Block size */
	unsigned int   s_log_frag_size;     /* Fragment size */
	unsigned int   s_blocks_per_group;  /* # Blocks per group */
	unsigned int   s_frags_per_group;   /* # Fragments per group */
	unsigned int   s_inodes_per_group;  /* # Inodes per group */
	unsigned int   s_mtime;             /* Mount time */
	unsigned int   s_wtime;             /* Write time */
	unsigned short s_mnt_count;         /* Mount count */
	unsigned short s_max_mnt_count;     /* Maximal mount count */
	unsigned short s_magic;             /* Magic signature */
	unsigned short s_state;             /* File system state */
	unsigned short s_errors;            /* Behaviour when detecting errors */
	unsigned short s_minor_rev_level;   /* minor revision level */
	unsigned int   s_lastcheck;         /* time of last check */
	unsigned int   s_checkinterval;     /* max. time between checks */
	unsigned int   s_creator_os;        /* OS */
	unsigned int   s_rev_level;         /* Revision level */
	unsigned short s_def_resuid;        /* Default uid for reserved blocks */
	unsigned short s_def_resgid;        /* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	unsigned int   s_first_ino;         /* First non-reserved inode */
	unsigned short s_inode_size;        /* size of inode structure */
	unsigned short s_block_group_nr;    /* block group # of this superblock */
	unsigned int   s_feature_compat;    /* compatible feature set */
	unsigned int   s_feature_incompat;  /* incompatible feature set */
	unsigned int   s_feature_ro_compat; /* readonly-compatible feature set */
	unsigned char  s_uuid[16];          /* 128-bit uuid for volume */
	char           s_volume_name[16];   /* volume name */
	char           s_last_mounted[64];  /* directory where last mounted */
	unsigned int   s_algorithm_usage_bitmap; /* For compression */
	/* 
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	unsigned char  s_prealloc_blocks;     /* Nr of blocks to try to preallocate*/
	unsigned char  s_prealloc_dir_blocks; /* Nr to preallocate for dirs */
	unsigned short s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	unsigned char  s_journal_uuid[16]; /* uuid of journal superblock */
	unsigned int   s_journal_inum;     /* inode number of journal file */
	unsigned int   s_journal_dev;      /* device number of journal file */
	unsigned int   s_last_orphan;      /* start of list of inodes to delete */
	unsigned int   s_hash_seed[4];     /* HTREE hash seed */
	unsigned char  s_def_hash_version; /* Default hash version to use */
	unsigned char  s_reserved_char_pad;
	unsigned short s_reserved_word_pad;
	unsigned int   s_default_mount_opts;
	unsigned int   s_first_meta_bg; /* First metablock block group */
	unsigned int   s_reserved[190]; /* Padding to the end of the block */
};


/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
	unsigned int   bg_block_bitmap;      /* Blocks bitmap block */
	unsigned int   bg_inode_bitmap;      /* Inodes bitmap block */
	unsigned int   bg_inode_table;       /* Inodes table block */
	unsigned short bg_free_blocks_count; /* Free blocks count */
	unsigned short bg_free_inodes_count; /* Free inodes count */
	unsigned short bg_used_dirs_count;   /* Directories count */
	/* The pad and reserved fields should be 0 for the assignment. */
	unsigned short bg_pad;
	unsigned int   bg_reserved[3];
};


/*
 * Structure of an inode on the disk
 */
struct ext2_inode {
	unsigned short i_mode;        /* File mode */
	/* Use 0 as the user id for the assignment. */
	unsigned short i_uid;         /* Low 16 bits of Owner Uid */
	unsigned int   i_size;        /* Size in bytes */
	/* You don't need to set access time for the assignment. */
	unsigned int   i_atime;       /* Access time */
	unsigned int   i_ctime;       /* Creation time */
	/* You don't need to set modification time for the assignment. */
	unsigned int   i_mtime;       /* Modification time */
	/* d_time must be set when appropriate */
	unsigned int   i_dtime;       /* Deletion Time */
	/* Use 0 as the group id for the assignment. */
	unsigned short i_gid;         /* Low 16 bits of Group Id */
	unsigned short i_links_count; /* Links count */
	unsigned int   i_blocks;      /* Blocks count IN DISK SECTORS*/
	/* You can ignore flags for the assignment. */
	unsigned int   i_flags;       /* File flags */
	/* You should set it to 0. */
	unsigned int   osd1;          /* OS dependent 1 */
	unsigned int   i_block[15];   /* Pointers to blocks */
	/* You should use generation number 0 for the assignment. */
	unsigned int   i_generation;  /* File version (for NFS) */
	/* The following fields should be 0 for the assignment.  */
	unsigned int   i_file_acl;    /* File ACL */
	unsigned int   i_dir_acl;     /* Directory ACL */
	unsigned int   i_faddr;       /* Fragment address */
	unsigned int   extra[3];
};


/*
 * Type field for file mode
 */
#define    EXT2_S_IFLNK  0xA000    /* symbolic link */
#define    EXT2_S_IFREG  0x8000    /* regular file */
#define    EXT2_S_IFDIR  0x4000    /* directory */
/* Other types, irrelevant for the assignment */
/* #define EXT2_S_IFSOCK 0xC000 */ /* socket */
/* #define EXT2_S_IFBLK  0x6000 */ /* block device */
/* #define EXT2_S_IFCHR  0x2000 */ /* character device */
/* #define EXT2_S_IFIFO  0x1000 */ /* fifo */


/*
 * Special inode numbers
 */
/* Root inode */
#define    EXT2_ROOT_INO         2
/* First non-reserved inode for old ext2 filesystems */
#define EXT2_GOOD_OLD_FIRST_INO 11


/*
 * Structure of a directory entry
 */
#define EXT2_NAME_LEN 255
/*
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext2_dir_entry {
	unsigned int   inode;     /* Inode number */
	unsigned short rec_len;   /* Directory entry length */
	unsigned char  name_len;  /* Name length */
	unsigned char  file_type;
	char           name[];    /* File name, up to EXT2_NAME_LEN */
};


/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
#define    EXT2_FT_UNKNOWN  0    /* Unknown File Type */
#define    EXT2_FT_REG_FILE 1    /* Regular File */
#define    EXT2_FT_DIR      2    /* Directory File */
#define    EXT2_FT_SYMLINK  7    /* Symbolic Link */
/* Other types, irrelevant for the assignment */
/* #define EXT2_FT_CHRDEV   3 */ /* Character Device */
/* #define EXT2_FT_BLKDEV   4 */ /* Block Device */
/* #define EXT2_FT_FIFO     5 */ /* Buffer File */
/* #define EXT2_FT_SOCK     6 */ /* Socket File */
#define    EXT2_FT_MAX      8



/********** New definitions **********/

unsigned char *disk;			// Global pointer to mmap the disk to
struct ext2_super_block *sb;	// Pointer to super block
struct ext2_group_desc *gt;		// Pointer to group table
struct ext2_inode *ind_tbl;		// Pointer to inode table
char *blk_bmp;					// Pointer to block bitmap
char *ind_bmp;					// Pointer to inode bitmap

// Used in rm, used to store previous dir entry of the entry to be removed
struct ext2_dir_entry *prev_dir_entry;

// Forward declarations
struct ext2_dir_entry *search_in_dir_inode(char *filename, int fnamelen, struct ext2_inode *dir);
struct ext2_dir_entry *search_in_dir_block(char *filename, int fnamelen, int block);
bool is_available_inode(int inode);
bool is_available_block(int block);

/**
 * Function for changing directory.
 * Assign inode of destination to result.
 * Arg1: 	Absolute path
 * Arg2:	Length of directory
 * Return: 	Success: 					inode number of directory
 * 			Path not exist: 			-ENOENT
 */			
int cd(char *dir, int dirlen) {
	// Set current working directory to root
	struct ext2_inode *cwd = ind_tbl + (EXT2_ROOT_INO - 1);

	// Recursively search subdirectories
	char *cursor = dir;
	int len;
	while(1) {

		// Make sure currently working on a directory
		assert(cwd -> i_mode & EXT2_S_IFDIR);

		// Get the length of next 'subdirectory'
		len = 0;
		while((cursor + len - dir) < dirlen && cursor[len] != '/') ++len;

		// If last directory
		if((cursor - dir + len) >= dirlen) {
			struct ext2_dir_entry *sub_dir_entry = search_in_dir_inode(cursor, len, cwd);

			// Subdirectory not found
			if(sub_dir_entry == NULL) return -ENOENT;

			return sub_dir_entry -> inode - 1;
		}

		// Not last directory, search recursively in subdirectories
		else {
			struct ext2_dir_entry *sub_dir_entry = search_in_dir_inode(cursor, len, cwd);

			// Subdirectory not found
			if(sub_dir_entry == NULL) return -ENOENT;

			// Update cwd pointing to inode of subdirectory
			cwd = ind_tbl + (sub_dir_entry -> inode - 1);
			// Update cursor to position of next subdirectory
			cursor += ++len;
		}	
	}
}


/**
 * Find file/directory in given directory's inode
 * Arg1:	Filename
 *			If NULL: search for empty slot
 * Arg2:	Filename length (without '\0')
			If NULL: min size for empty slot	
 * Arg3:	Inode of directory to search
 * Return:	Success:	corresponding struct ext2_dir_entry *
 *			Fail: 		NULL
 */
 struct ext2_dir_entry *search_in_dir_inode(char *filename, int fnamelen, struct ext2_inode *dir) {

	// Traverse current inode's i_block to find subdirectory
	int i, j, k;
	struct ext2_dir_entry *rv = NULL;

	for(i = 0; i < 12; ++i) {	// Direct blocks
		
		// Value 0 suggests no further block defined, i.e. not found
		if(filename != NULL && (dir -> i_block)[i] == 0) {
			return NULL;
		}

		// i_block's block index starts from 0
		rv = search_in_dir_block(filename, fnamelen, (dir -> i_block)[i]);	
		if(rv) return rv;	// Found in direct block
	}

	if(rv == 0) {	// If not found, search in indirect blocks (*256)
		int *indirect_block = (int *)(disk + (dir -> i_block)[12] * EXT2_BLOCK_SIZE);
		
		for(i = 0; i < 256; ++i) {
			
			// Value 0 suggests no further block defined, i.e. not found
			if(indirect_block[i] == 0) {
				return NULL;
			}

			rv = search_in_dir_block(filename, fnamelen, indirect_block[i]);
			if(rv) return rv;	// Found in indirect block
		}
	}

	if(rv == 0) {	// If not found in indirect block, search in double indirect block
		int *indirect_block = (int *)(disk + (dir -> i_block)[13] * EXT2_BLOCK_SIZE);

		for(i = 0; i < 256; ++i) {
			int *db_indirect_block = (int *)(disk + indirect_block[i] * EXT2_BLOCK_SIZE);
			
			for(j = 0; j < 256; ++j) {

				// Value 0 suggests no further block defined, i.e. not found
				if(db_indirect_block[j] == 0) {
					return NULL;
				}

				rv = search_in_dir_block(filename, fnamelen, db_indirect_block[j]);
				if(rv) return rv;	// Found in double indirect block
			}
		}
	}

	if(rv == 0) {	// If not found in double indirect block, search in triple indirect block
		int *indirect_block = (int *)(disk + (dir -> i_block)[14] * EXT2_BLOCK_SIZE);

		for(i = 0; i < 256; ++i) {
			int *db_indirect_block = (int *)(disk + indirect_block[i] * EXT2_BLOCK_SIZE);

			for(j = 0; j < 256; ++j) {
				int *tp_indirect_block = (int *)(disk + db_indirect_block[j] * EXT2_BLOCK_SIZE);

				for(k = 0; k < 256; ++k) {

					// Value 0 suggests no further block defined, i.e. not found
					if(tp_indirect_block[k] == 0) {
						return NULL;
					}		
					
					rv = search_in_dir_block(filename, fnamelen, tp_indirect_block[k]);
					if(rv) return rv;	// Found in triple indirect block
				}
			}
		}
	}

	// Not found in all 16843020 blocks (which is unlikely :) lol)
	return NULL;
}


/**
 * Search for a file/dir or slot in a data block.
 * Arg1: 	Filename (char buffer array)
			If NULL: search for next empty slot, and set context in dir block
 * Arg2:	Length of filename (without '\0')
			If Arg1 NULL: min size for empty slot
 * Arg3:	Block number (begins from 0)
 * Return: 	Success: corresponding ext2_dir_entry *
 * 			Fail: NULL
 */
struct ext2_dir_entry *search_in_dir_block(char *filename, int fnamelen, int block) {

	unsigned char *block_p = disk + block * EXT2_BLOCK_SIZE;		// Start of block
	unsigned char *cur = block_p;									// Search pos
	prev_dir_entry = NULL;			// If return with NULL, then first ent in this dir block

	// If this is a newly allocated block
	if(filename == NULL && (cur - disk) % 1024 == 0 && ((struct ext2_dir_entry *)(cur)) -> rec_len == 0) {
		struct ext2_dir_entry *p = (struct ext2_dir_entry *)cur;
		return p;
	}

	// Get current directory size, because different with rec_len for last dir block
	int cur_dir_size, prev_dir_size;
	while(1) {

		prev_dir_size = cur_dir_size;
		cur_dir_size = 8 + ((struct ext2_dir_entry *)(cur)) -> name_len;
		cur_dir_size += 3; cur_dir_size >>= 2; cur_dir_size <<= 2;

		if(filename == NULL) {	// Search for empty slot with enough size
			
			// Found enough space in this block

			if(((struct ext2_dir_entry *)(cur)) -> rec_len - cur_dir_size >= fnamelen) {
				// Update rec_len of current block
				((struct ext2_dir_entry *)(cur + cur_dir_size)) -> rec_len
					= ((struct ext2_dir_entry *)(cur)) -> rec_len - cur_dir_size;
				// Update rec_len of previous block
				((struct ext2_dir_entry *)(cur)) -> rec_len = cur_dir_size;
				
				return ((struct ext2_dir_entry *)(cur + cur_dir_size));
			}
		}

		else{	// Search for file/dir
			// Found if name and namelength are both equal
			if(fnamelen == ((struct ext2_dir_entry *)(cur)) -> name_len && 
				strncmp(((struct ext2_dir_entry *)(cur)) -> name, filename, fnamelen) == 0) {
				return (struct ext2_dir_entry *)cur;
			}
		}

		// Update cur to position of next file in this block
		prev_dir_entry = (struct ext2_dir_entry *)cur;	// Store previous entry
		if(filename != NULL) {
			cur += ((struct ext2_dir_entry *)(cur)) -> rec_len;
		}
		else {
			cur += cur_dir_size;
		}

		// Not found if p reach end of block
		if(cur - block_p >= EXT2_BLOCK_SIZE) return NULL;
	}
}


/**
 * Returns type(short) field given an inode index
 */
unsigned short get_inode_mode(int inode_idx) {

	// Make sure inode index in range
	assert(inode_idx <= sb -> s_inodes_count);
	
	return (ind_tbl + inode_idx - 1) -> i_mode;
	
}

/**
 * Returns whether inode is available in bitmap.
 * @arg1: inode number (starting from 1)
 */
bool is_available_inode(int inode) {
	--inode;
	return (ind_bmp[inode >> 3] & (1 << (inode % 8))) == 0;
}

/**
 * Returns whether a block is available in bitmap.
 * @arg1: block number (starting from 0)
 */
 bool is_available_block(int block) {
	return (blk_bmp[block >> 3] & (1 << (block % 8))) == 0;
}

/**
 * Allocate a new block of inode with smallest index
 * Set all variables in new inode to zero
 * Return:		Success:	Index of new inode (>0)
 * 				Fail: 		0
 */
int allocate_inode() {
	int i;
	for(i = 11; i < sb -> s_inodes_count; ++i) {	// Skip reserved inodes
		if((ind_bmp[i >> 3] & (1 << (i % 8))) == 0) {
			ind_bmp[i >> 3] |= (1 << (i % 8));
			memset(ind_tbl + i, 0, sizeof(struct ext2_inode));
			fprintf(stderr, "Allocating inode %d\n", i + 1);
			return i + 1;
		}
	}

	// Update block group desc table & super block
	sb -> s_free_inodes_count -= 1;
	gt -> bg_free_inodes_count -= 1;

	return 0;
}

/**
 * Allocate a new block of data with smallest index
 * Set all variables in new inode to zero
 * Return:		Success:	Index of new inode (>0)
 * 				Fail: 		0
 */
int allocate_block() {
	int i;
	for(i = 8; i < sb -> s_blocks_count; ++i) {		// Skip inode table stuff
		if((blk_bmp[i >> 3] & (1 << (i % 8))) == 0) {
			blk_bmp[i >> 3] |= (1 << (i % 8));
			memset(disk + i * EXT2_BLOCK_SIZE, 0, EXT2_BLOCK_SIZE);
			fprintf(stderr, "Allocating block %d\n", i + 1);
			return i + 1;
		}
	}

	// Update block group desc table & super block
	sb -> s_free_blocks_count -= 1;
	gt -> bg_free_blocks_count -= 1;

	return 0;
}
#endif