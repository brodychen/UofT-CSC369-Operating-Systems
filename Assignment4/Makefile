TARGET = ext2_mkdir ext2_cp ext2_rm
DEPS = ext2.h
CFLAGS =  -g

all: ${TARGET}

ext2_mkdir: ext2_mkdir.c ${DEPS}
	gcc -o ext2_mkdir ext2_mkdir.c ${CFLAGS}

ext2_cp: ext2_cp.c ${DEPS}
	gcc -o ext2_cp ext2_cp.c ${CFLAGS}

ext2_rm: ext2_rm.c ${DEPS}
	gcc -o ext2_rm ext2_rm.c ${CFLAGS}

clean:
	rm -f ${TARGET}