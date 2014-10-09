/**
 *  @file vga2ppm.c
 *
 *  A small standalone converter program to convert the custom VGA format to PPM.
 *
 *  The PPM format from the Netpbm standard is a very simple image format. While it is somewhat
 *  obscure it makes for a reasonably standardised basis for further conversion.
 *
 *  This program is too detached from Wolfensein 3D to be part of the extractor utility, but at
 *  the same time it is also too small and specialised to have its own repository. The format of
 *  the VGA file is two unsigned 16-bit integers for the width and height respectively followed
 *  by a linear sequence of pixels. These pixels need to be mapped to RGB values and ordered
 *  properly for the output file.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/** Structure representing a 32-bit RGBA colour. */
struct color {
	uint8_t r; ///< Red
	uint8_t g; ///< Green
	uint8_t b; ///< Blue
	uint8_t a; ///< Alpha (unused)
};

/** Maps three integer numbers to a colour struct. */
#define RGB(r, g, b) {(r)*255/63, (g)*255/63, (b)*255/63, 0xFF}

/** Map an index number to an RGBA colour. */
struct color wolfenstein_palette[] = {
	#include "palette/wolfenstein_palette.txt"
};

int main(int argc, char *argv[]) {
	uint16_t  width, height; // Dimensions of the image
	uint8_t  *pixels;        // Variable holding the individual pixels of the image.

	// The first two numbers are the image size, read and store them
	fseek(stdin, 0, SEEK_SET);
	fread(&width,  sizeof(uint16_t), 1, stdin);
	fread(&height, sizeof(uint16_t), 1, stdin);

	// Allocate enough space to hold the pixels and then read them linearly
	if ((pixels = malloc(width * height * sizeof *pixels)) == NULL) {
		fprintf (stderr, "Error: could not allocte memory to hold image.\n");
		return 1;
	}
	fread(pixels, sizeof(uint8_t), width * height, stdin);

	// The header of the file, contains the magic number, a comment, the image size and the largest value per channel.
	fprintf(stdout, "P6\n%s\n%d %d\n255\n", "#This file follows the binary PPM Format from the Netpbm standard", width, height);

	// Iterate indefinitely over the pixel array and write it to the output stream.
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			// This is how the pixels need to be read to order them linearly in the output picture.
			fwrite(&wolfenstein_palette[pixels[(j*(width>>2)+(i>>2))+(i&3)*(width>>2)*height]], 3 * sizeof(uint8_t), 1, stdout);
		}
	}
	return 0;
}

