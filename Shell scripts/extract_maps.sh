#!/bin/sh

mkdir -p "Maps"
for e in {1..6}
do
	for l in {1..10}
	do
		for m in {0..2}
		do
			./wolf3dextract -lm $e $l $m >"Maps/e${e}_l${l}_m${m}.w3dmap"
		done
	done
done
