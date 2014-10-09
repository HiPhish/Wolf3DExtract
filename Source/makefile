CC = clang
# turn on  level 2 optimisation and Clang hard-mode
CFLAGS = -std=c99 -O2 -pedantic-errors -Werror

all: debug clean

globals.o: globals.c globals.h
	${CC} ${CFLAGS} -c  globals.c

compression.o: compression/compression.c compression/compression.h
	${CC} ${CFLAGS} -c  compression/compression.c

map_extract.o: map_extract/map_extract.c map_extract/map_extract.h
	${CC} ${CFLAGS} -c map_extract/map_extract.c

pic_extract.o: pic_extract/pic_extract.c pic_extract/pic_extract.h
	${CC} ${CFLAGS} -c pic_extract/pic_extract.c

wolf3dextract.o: wolf3dextract.c
	${CC} ${CFLAGS} -c  wolf3dextract.c

release: wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o
	${CC} ${CFLAGS} wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o -o wolf3dextract
	${CC} ${CFLAGS} rgb2ppm.c -o rgb2ppm

debug: wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o
	${CC} ${CFLAGS} -DDEBUG wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o -o wolf3dextract
	${CC} ${CFLAGS} vga2ppm.c -o vga2ppm

clean:
	rm -rf *.o
