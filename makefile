# Call the makefile one level deeper and move the result back up here
all: source/makefile
	cd source; make; mv wolf3dextract ../wolf3dextract
