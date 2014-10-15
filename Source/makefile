CC = clang
# turn on  level 2 optimisation and Clang hard-mode (error on language extensions and treat warnings as errors)
CFLAGS = -std=c99 -O2 -pedantic -Werror

all: release clean

globals.o: globals.c globals.h
	${CC} ${CFLAGS} -c  globals.c

compression.o: compression/compression.c compression/compression.h
	${CC} ${CFLAGS} -c  compression/compression.c

map_extract.o: map_extract/map_extract.c map_extract/map_extract.h
	${CC} ${CFLAGS} -c map_extract/map_extract.c

pic_extract.o: pic_extract/pic_extract.c pic_extract/pic_extract.h
	${CC} ${CFLAGS} -c pic_extract/pic_extract.c

sprite_extract.o: sprite_extract/sprite_extract.c sprite_extract/sprite_extract.h
	${CC} ${CFLAGS} -c sprite_extract/sprite_extract.c

audio_extract.o: audio_extract/audio_extract.c audio_extract/audio_extract.h
	${CC} ${CFLAGS} -c audio_extract/audio_extract.c

wolf3dextract.o: wolf3dextract.c
	${CC} ${CFLAGS} -c  wolf3dextract.c

release: wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o sprite_extract.o audio_extract.o vga2ppm.c pcs2wav.c
	${CC} ${CFLAGS} wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o sprite_extract.o audio_extract.o -o wolf3dextract
	${CC} ${CFLAGS} vga2ppm.c -o vga2ppm
	${CC} ${CFLAGS} pcs2wav.c -o pcs2wav

clean:
	rm -rf *.o
