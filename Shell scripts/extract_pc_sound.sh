#!/bin/sh

mkdir -p PC Sounds
for i in {0..435}
do
	./wolf3dextract -snd $i pc >sound_pc_$i.ifs
done
