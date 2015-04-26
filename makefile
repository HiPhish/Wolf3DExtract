# Call the makefile one level deeper and move the result back up here
all: source/makefile
	cd source; make;
	mv wolf3dextract ../wolf3dextract
	mv vgs2ppm ../vgs2ppm
	mv snd2wav ../snd2wav
	mv snd2wlf ../snd2wlf

