#!/bin/sh

mkdir -p Music
for i in {0..26}
do
	./wolf3dextract -mus $i >Music/music_$i.wlf
done
