#!/bin/sh

mkdir -p "Digi Sounds"
for i in {0..45}
do
	./wolf3dextract -snd $i digi | ./snd2wav -d >"Digi Sounds/sound_digi_$i.wav"
done
