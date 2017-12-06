TARGET = ext2_mkdir ext2_cp ext2_rm ext2_rm_bonus ext2_restore ext2_ln ext2_checker
DEPS = ext2_helper.h ext2_utl.c
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

ext2_ln: ext2_ln.c ${DEPS}
	gcc -o ext2_ln ext2_ln.c ${CFLAGS}

ext2_checker: ext2_checker.c ${DEPS}
	gcc -o ext2_checker ext2_checker.c ${CFLAGS}

clean:
	rm -f ${TARGET}