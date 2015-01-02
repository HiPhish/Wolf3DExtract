#!/bin/sh

mkdir -p "AdLib Sounds"
for i in {0..86}
do
	./wolf3dextract -snd $i adlib >"AdLib Sounds/sound_adlib_$i.adlib"
done
