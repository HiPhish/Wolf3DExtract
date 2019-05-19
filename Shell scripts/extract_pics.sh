#!/bin/sh

mkdir -p pics
for i in {3..134}
do
	./wolf3dextract -pic $i | ./vga2ppm --woven | convert /dev/stdin pics/pic_$i.png
done
