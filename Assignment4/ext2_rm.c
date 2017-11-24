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
 * This program works like rm.
 * 
 * @arg1: An ext2 formatted virtual disk
 * (@arg2: -r flag)
 * @arg3: Absolute path to file/link
 *
 * @return: 	Success: 	0
 */

int main() {

}

