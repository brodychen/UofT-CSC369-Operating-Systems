TARGET = ext2_mkdir
DEPS = ext2.h
CFLAGS =  -g

all: ${TARGET}

ext2_mkdir: ext2_mkdir.c ${DEPS}
	gcc -o ext2_mkdir ext2_mkdir.c ${CFLAGS}

clean:
	rm -f ${TARGET}