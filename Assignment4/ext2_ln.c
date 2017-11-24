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