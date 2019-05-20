#!/bin/sh

mkdir -p sprites
for i in {0..435}
do
	./wolf3dextract -spr $i | ./vga2ppm --flipped | convert /dev/stdin sprites/sprite_$i.png
done
