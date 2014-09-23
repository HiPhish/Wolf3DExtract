CC = clang
# turn on  level 2 optimisation and Clang hard-mode
CFLAGS = -std=c99 -O2 -pedantic-errors -Werror

all: release clean

globals.o: globals.c globals.h
	${CC} ${CFLAGS} -c  globals.c

compression.o: compression/compression.c compression/compression.h
	${CC} ${CFLAGS} -c  compression/compression.c

map_extract.o: map_extract/map_extract.c map_extract/map_extract.h
	${CC} ${CFLAGS} -c map_extract/map_extract.c

wolf3dextract.o: wolf3dextract.c
	${CC} ${CFLAGS} -c  wolf3dextract.c

release: wolf3dextract.o globals.o compression.o map_extract.o
	${CC} ${CFLAGS} wolf3dextract.o globals.o compression.o map_extract.o -o wolf3dextract

debug: wolf3dextract.o globals.o compression.o map_extract.o
	${CC} ${CFLAGS} -DDEBUG wolf3dextract.o globals.o compression.o map_extract.o -o wolf3dextract

clean:
	rm -rf *.o
