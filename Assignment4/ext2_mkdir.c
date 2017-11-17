#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>

#include "ext2.h"

/**
 * This program works like mkdir, creating the final directory on the 
 * specified path on the disk.
 *
 * Args1: An ext2 formatted virtual disk
 * Args2: An absolute path on this disk
 *
 * Return: 	Success:					0
 * 			Path not exist: 			ENOENT
 * 			Directory already exists: 	EEXIST
 */


int main() {

	
	
}