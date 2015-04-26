# Turn on  Clang hard-mode (error on language extensions and treat warnings as
# errors). Always use debug symbols.
$(CC)   = clang
CFLAGS  = -std=c99 -g -pedantic -Werror -Wall
OBJECTS = globals.o        \
          compression.o    \
          map_extract.o    \
          pic_extract.o    \
          sprite_extract.o \
          audio_extract.o  \
          wolf3dextract.o

PROGS   = wolf3dextract \
          vga2ppm       \
          snd2wav       \
          snd2wlf

#===[ TARGETS ]=================================================================

# Unoptimised build for debugging.
debug: CFLAGS += -O0
debug: $(PROGS)

# Optimised build for release.
release: CFLAGS += -O2
release: $(PROGS)

# Run static analyser.
analyze: CFLAGS += --analyze
analyze: $(OBJECTS)

# Alternate spelling.
analyse: analyze

globals.o: globals.c globals.h
	@$(CC) ${CFLAGS} -c  globals.c

compression.o: compression/compression.c compression/compression.h
	@$(CC) ${CFLAGS} -c  $*/$*.c

map_extract.o: map_extract/map_extract.c map_extract/map_extract.h
	@$(CC) ${CFLAGS} -c $*/$*.c

pic_extract.o: pic_extract/pic_extract.c pic_extract/pic_extract.h
	@$(CC) ${CFLAGS} -c $*/$*.c

sprite_extract.o: sprite_extract/sprite_extract.c sprite_extract/sprite_extract.h
	@$(CC) ${CFLAGS} -c $*/$*.c

audio_extract.o: audio_extract/audio_extract.c audio_extract/audio_extract.h
	@$(CC) ${CFLAGS} -c $*/$*.c

wolf3dextract.o: wolf3dextract.c
	@$(CC) ${CFLAGS} -c  wolf3dextract.c

#---[ Program targets ]---------------------------------------------------------

wolf3dextract: $(OBJECTS)
	@$(CC) ${CFLAGS} -o wolf3dextract $(OBJECTS)

vga2ppm: vga2ppm.c
	@$(CC) ${CFLAGS} vga2ppm.c -o vga2ppm

snd2wav: snd2wav.c
	@$(CC) ${CFLAGS} snd2wav.c -o snd2wav

snd2wlf: snd2wlf.c
	@$(CC) ${CFLAGS} snd2wlf.c -o snd2wlf

#---[ Phony targets ]-----------------------------------------------------------

clean:
	rm -rf *.o *.plist $(PROGS)
ctags:
	@ctags -R

