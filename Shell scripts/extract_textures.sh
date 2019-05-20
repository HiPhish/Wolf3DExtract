#!/bin/sh

mkdir -p textures
for i in {0..105}
do
	./wolf3dextract -tex $i | ./vga2ppm --transposed | convert /dev/stdin textures/texture_$i.png
done
