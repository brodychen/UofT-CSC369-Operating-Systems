TARGET = ext2_mkdir ext2_cp
DEPS = ext2.h
CFLAGS =  -g

all: ${TARGET}

ext2_mkdir: ext2_mkdir.c ${DEPS}
	gcc -o ext2_mkdir ext2_mkdir.c ${CFLAGS}

ext2_cp: ext2_cp.c ${DEPS}
	gcc -o ext2_cp ext2_cp.c ${CFLAGS}

clean:
	rm -f ${TARGET}