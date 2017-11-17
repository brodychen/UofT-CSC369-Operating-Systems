DEPS = ext2.h

readimage: readimage.c ${DEPS}
	gcc -o readimage readimage.c

debug: readimage.c ${DEPS}
	gcc -g -o readimage readimage.c

clean:
	rm -f readimage