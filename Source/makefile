# turn on  level 2 optimisation and Clang hard-mode (error on language
# extensions and treat warnings as errors).
CFLAGS = -std=c99 -O2 -pedantic -Werror

all: release clean

globals.o: globals.c globals.h
	cc ${CFLAGS} -c  globals.c

compression.o: compression/compression.c compression/compression.h
	cc ${CFLAGS} -c  compression/compression.c

map_extract.o: map_extract/map_extract.c map_extract/map_extract.h
	cc ${CFLAGS} -c map_extract/map_extract.c

pic_extract.o: pic_extract/pic_extract.c pic_extract/pic_extract.h
	cc ${CFLAGS} -c pic_extract/pic_extract.c

sprite_extract.o: sprite_extract/sprite_extract.c sprite_extract/sprite_extract.h
	cc ${CFLAGS} -c sprite_extract/sprite_extract.c

audio_extract.o: audio_extract/audio_extract.c audio_extract/audio_extract.h
	cc ${CFLAGS} -c audio_extract/audio_extract.c

wolf3dextract.o: wolf3dextract.c
	cc ${CFLAGS} -c  wolf3dextract.c

debug: wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o sprite_extract.o audio_extract.o vga2ppm.c snd2wav.c snd2wlf.c
	cc ${CFLAGS} wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o sprite_extract.o audio_extract.o -o wolf3dextract
	cc ${CFLAGS} vga2ppm.c -o vga2ppm
	cc ${CFLAGS} snd2wav.c -o snd2wav
	cc ${CFLAGS} snd2wlf.c -o snd2wlf

release: wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o sprite_extract.o audio_extract.o vga2ppm.c snd2wav.c snd2wlf.c
	cc ${CFLAGS} wolf3dextract.o globals.o compression.o map_extract.o pic_extract.o sprite_extract.o audio_extract.o -o wolf3dextract
	cc ${CFLAGS} vga2ppm.c -o vga2ppm
	cc ${CFLAGS} snd2wav.c -o snd2wav
	cc ${CFLAGS} snd2wlf.c -o snd2wlf
	rm -rf *.o

clean:
	rm -rf *.o
