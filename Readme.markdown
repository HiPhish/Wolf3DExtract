Wolfenstein 3D Extractor - wolf3dextract
========================================

Wolfenstein 3D Extractor is a command-line utility for extracting assets from the data files of the PC game Wolfenstein 3D.
The program is written more as a proof-of-concept than an "everyman's tool"; this means you will need to call it from the
command line and supply exactly which assets you want. The output is printed to the standard output, so you will want to
redirect it or pipe it into another program.

Compiling
---------
Care has been taken to make sure the code fully complies with the C99 standard, so any compiler that conforms should be able
to compile the code. For users of Clang a makefile is included.

Usage
-----
Execute the program from the same directory where your data files are. Pass it the folliwing argumenst:

	-lh  e l      Extract header of level `l` from episode `e`
	-l   e l m    Extract map `m` of level `l` from episode `e`
	-ext abc      Set extension of data files to the string "abc"
	
Arguments are processed in the order they are given, so it is possible for example to change the file extension midway
during execution. Example:

	wolf3dextrac -ext WL6 -lh 5 2 -ext WL1 -lh 1 2

Extracts first the header of episode 5, level 2 from the registered version, then episode 1, level 2 from the shareware version.

Limitations & bugs
------------------
Currently the program can only extract maps, more types will be added as the formats are understood. Also, currently the
output contains the padding from platform-specific data structure alignment; this will have to be filtered out.
