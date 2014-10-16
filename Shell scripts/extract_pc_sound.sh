#!/bin/sh

mkdir -p "PC Sounds"
for i in {0..86}
do
	./wolf3dextract -snd $i pc | ./snd2wav -p >"PC Sounds/sound_pc_$i.wav"
done
