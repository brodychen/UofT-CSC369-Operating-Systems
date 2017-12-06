TARGET = ext2_mkdir ext2_cp ext2_rm ext2_rm_bonus ext2_restore
DEPS = ext2_helper.h
CFLAGS = -g

all: ${TARGET}

ext2_mkdir: ext2_mkdir.c ${DEPS}
	gcc -o ext2_mkdir ext2_mkdir.c ${CFLAGS}

ext2_cp: ext2_cp.c ${DEPS}
	gcc -o ext2_cp ext2_cp.c ${CFLAGS}

ext2_rm: ext2_rm.c ${DEPS}
	gcc -o ext2_rm ext2_rm.c ${CFLAGS}

ext2_rm_bonus: ext2_rm_bonus.c ${DEPS}
	gcc -o ext2_rm_bonus ext2_rm_bonus.c ${CFLAGS}

ext2_restore: ext2_restore.c ${DEPS}
	gcc -o ext2_restore ext2_restore.c ${CFLAGS}

clean:
	rm -f ${TARGET}