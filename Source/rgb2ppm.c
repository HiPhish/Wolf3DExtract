/**
 *  @file rgb2ppm.c
 *
 *  A small standalone converter program to convert RGB to PPM.
 *
 *  The PPM format from the Netpbm standard is a very simple image format. While it is somewhat
 *  obscure it make for a reasonably standardised basis for further conversion.
 *
 *  This program is too detached from Wolfensein 3D to be part of the extractor utility, but at
 *  the same time it is also too small and specialised to be its own repository.
 */


#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
	uint8_t byte;           // Variable holding the individual bytes from the input
	uint16_t width, height; // Dimeonsions of the image

	fseek(stdin, 0, SEEK_SET);
	fread(&width,  sizeof(uint16_t), 1, stdin);
	fread(&height, sizeof(uint16_t), 1, stdin);

	// The header of the file, contains the magic number, comment and image size.
	fprintf(stdout, "P6\n%s\n%d %d\n255\n", "#This file follows the binary PPM Format from the Netpbm standard", width, height);

	// Iterate indefinitely over the input stream and write it to the output stream.
	while (1) {
		// At least until hitting the end of file.
		if(fread(&byte, sizeof(uint8_t), 1, stdin) < sizeof(uint8_t)) {
			break;
		}
		fwrite(&byte, sizeof(uint8_t), 1, stdout);
	}
	return 0;
}
