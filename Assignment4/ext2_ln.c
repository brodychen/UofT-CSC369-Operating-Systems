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

#include "ext2_utl.c"

/**
 * This program creates a link between two files.
 *
 * @arg1: An ext2 formatted virtual disk
 * @arg2: Absolute path of source file
 * @arg3: Absolute path of destination path
 *
 * @return: 	Success:			0
 * 				File not exists: 	ENOENT
 *				Link already exists:	EEXIST
 *				Refers to a directory: 	EISDIR
 */
extern unsigned char *disk;			// Global pointer to mmap the disk to
 
extern struct ext2_super_block *sb;	// Pointer to super block
extern struct ext2_group_desc *gt;		// Pointer to group table
extern struct ext2_inode *ind_tbl;		// Pointer to inode table
extern char *blk_bmp;					// Pointer to block bitmap
extern char *ind_bmp;					// Pointer to inode bitmap

int main(int argc, char **argv) {

	// if(argc != 4) {
	// 	fprintf(stderr, "Usage: %s <image file name> <path on OS> <path on image>\n", argv[0]);
	// 	exit(1);
	// }
	// int fd = open(argv[1], O_RDWR);

	// // Map disk to memory, allow r & w
	// // Share this mapping between processes
	// disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	// if(disk == MAP_FAILED) {
	// 	perror("mmap");
	// 	exit(1);
	// }

	// FILE *fp;
	// if((fp = fopen(argv[2], "r")) == NULL) {
	// 	fprintf(stderr, "Can't open input file %s\n", argv[2]);
	// 	exit(1);
	// }

	// sp_blk = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	// grp_tbl = (struct ext2_group_desc *)(disk + EXT2_BLOCK_SIZE);
	// ind_tbl = (struct ext2_inode*)(disk + EXT2_BLOCK_SIZE * gt[0].bg_inode_table);
	// blk_bmp = disk + EXT2_BLOCK_SIZE * gt[0].bg_block_bitmap;
	// ind_bmp = disk + EXT2_BLOCK_SIZE * gt[0].bg_inode_bitmap;

	// int i = strlen(argv[2]) - 1;
	// if(argv[2][i] == '/') argv[2][i--] = '\0';
	// if(argv[2][i] == '.' && argv[2][i] = '/'){
	// 	argv[2] += 2;
	// 	i -= 2;
	// }
	// ++i;

	// int j = strlen(argv[3] - 1);
	// if(argv[3][i] == '/') argv[3][i--] = '\0';
	// if(argv[3][i] == '.' && argv[3][i] = '/'){
	// 	argv[3] += 2;
	// 	i -= 2;
	// }
	// ++j;

	// int parent_dir_inode = EXT2_ROOT_INO - 1;			

	// if(argv[2][0] != '.') {
	// 	parent_dir_inode = cd(argv[2], i);

	// 	if(parent_dir_inode == -ENOENT) {
	// 		fprintf(stderr, "cannot cp: No such file or directory\n");
	// 		exit(ENOENT);
	// 	}
	// }

	// if(argv[3][0] != '.') {
	// 	parent_dir_inode = cd(argv[3], j);

	// 	if(parent_dir_inode == -ENOENT) {
	// 		fprintf(stderr, "cannot cp: No such file or directory\n");
	// 		exit(ENOENT);
	// 	}
	// }

	
	char* filename = NULL;
	char* path = NULL;
	
	if (argc != 4){
		perror("usage: <image file> <source path> <target path>"); //print usage
		exit(-1);
	}

	int fd = open(argv[1], O_RDWR);

	disk = mmap(NULL, 128*1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
	if(disk == MAP_FAILED){ //mmap returns MAP_FAILED if memory mapping fails
		perror("Memory Mapping Error");
		exit(-1);
	}

	char *link = argv[2];
	char *file = argv[3]; 

	if( strlen(argv[2]) < 2 ||  strlen(argv[2]) < 2){
		perror("Iconrrent path entered");
		exit(1);
	}

	struct path *file_path = divide_path(file); 
	struct path *exist_link = divide_path(link);
	
	filename = return_last_entry(link);
	path = return_path(link);

	if(filename == NULL){
		perror(" invaled link path (source)");
		return ENOENT;
	}
	
	int parent_idx; 
	struct path* link_p_path; 
	if(path == NULL){	
		parent_idx = 2; 
	}else{
		link_p_path = divide_path(path); 
		parent_idx = locate_directory(link_p_path); 
	int exist_idx = locate_directory(exist_link); 
    free_path(link_p_path); 

	if(exist_idx > 0){ 
		perror("link already exist");
		return EEXIST;
	}
	free_path(exist_link); 


	if( file_path == NULL){ 
		perror("target file not found");
		exit(1);
	}
	
    int link_idx = locate_directory(file_path);

	if(link_idx < 1){ 
		perror("target file not found");
		return ENOENT;
	}

	struct ext2_inode *link_inode = find_inode(link_idx); 
	if(link_inode->i_mode & EXT2_S_IFDIR){
		perror("target file is dir");
		return EISDIR;
	}
     
    if((parent_idx) < 1){
    	perror("target file not found");
		return ENOENT;
    }

    struct ext2_dir_entry dir;
    dir.file_type = EXT2_FT_REG_FILE;
    dir.inode = link_idx;
    char *dir_name = dir.name;
    strncpy(dir_name, filename , strlen(filename));
    dir.name[strlen(filename)] = '\0';
    dir.name_len = strlen(filename);
    dir.rec_len = 0;

    insert_dir_entry(parent_idx, &dir);
 	
    free_path(file_path);
    free(path);
    free(filename);
    return 0;

}