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
#include <string.h>

// Modes to assemble the image from the pixel bytes.
#define LINEAR_MODE     0 ///< Assemble the pixels in the order they were given (useless, but intereting results).
#define WOVEN_MODE      1 ///< Weave the pixels for bitmap pictures (default).
#define TRANSPOSED_MODE 2 ///< Linear but transposed (for textures).
#define FLIPPED_MODE    3 ///< Flipped horizontally (for sprites).
#define NUMBER_OF_MODES 4 ///< Number of possible assembly modes.

/** Maps three integer numbers to a colour struct (full alpha component). */
#define RGB(r, g, b) {(r)*255/63, (g)*255/63, (b)*255/63, 0xFF}

/** Structure representing a 32-bit RGBA colour. */
struct color {
	uint8_t r; ///< Red
	uint8_t g; ///< Green
	uint8_t b; ///< Blue
	uint8_t a; ///< Alpha (unused)
};

/** Assemble the pixels strictly as they were given. */
void assemble_linear(uint16_t width, uint16_t height, uint8_t *pixels);

/** Assemble the pixels VGA-style by "weaving" them together. */
void assemble_woven(uint16_t width, uint16_t height, uint8_t *pixels);

/** Assemble the pixel matrix transposed (for textures). */
void assemble_transposed(uint16_t width, uint16_t height, uint8_t *pixels);

/** Assemble the pixels flipped horizontally (for sprites). */
void assemble_flipped(uint16_t width, uint16_t height, uint8_t *pixels);

void print_usage(void);

/** Map an assembly mode to an assembly function. */
void (*assembly_function[NUMBER_OF_MODES])(uint16_t width, uint16_t height, uint8_t *pixels) = {
	[LINEAR_MODE    ] = assemble_linear,     ///< Linear.
	[WOVEN_MODE     ] = assemble_woven,      ///< Woven.
	[TRANSPOSED_MODE] = assemble_transposed, ///< Transposed.
	[FLIPPED_MODE   ] = assemble_flipped,    ///< Flipped.
};

/** Currently selected assembly mode. */
int assembly_mode = TRANSPOSED_MODE;

/** Map an index number to an RGBA colour. */
struct color wolfenstein_palette[] = {
	#include "palette/wolfenstein_palette.h"
};

void process_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	process_arguments(argc, argv);

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
	assembly_function[assembly_mode](width, height, pixels);

	return 0;
}

void assemble_linear(uint16_t width, uint16_t height, uint8_t *pixels) {
	for (int i = 0; i < width * height; ++i) {
			fwrite(&wolfenstein_palette[pixels[i]], 3 * sizeof(uint8_t), 1, stdout);
	}
}

void assemble_woven(uint16_t width, uint16_t height, uint8_t *pixels) {
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			// This is how the pixels need to be read to order them linearly in the output picture.
			fwrite(&wolfenstein_palette[pixels[(j*(width>>2)+(i>>2))+(i&3)*(width>>2)*height]], 3 * sizeof(uint8_t), 1, stdout);
		}
	}
}

void assemble_transposed(uint16_t width, uint16_t height, uint8_t *pixels) {
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			fwrite(&wolfenstein_palette[pixels[i*height + j]], 3 * sizeof(uint8_t), 1, stdout);
		}
	}
}

void assemble_flipped(uint16_t width, uint16_t height, uint8_t *pixels) {
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			fwrite(&wolfenstein_palette[pixels[(height-1 - j) * width + i]], 3 * sizeof(uint8_t), 1, stdout);
		}
	}
}

void process_arguments(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i],        "-help"      ) == 0) {
			print_usage();
		} else if (strcmp(argv[i], "-linear"    ) == 0) {
			assembly_mode = WOVEN_MODE;
		} else if (strcmp(argv[i], "-woven"     ) == 0) {
			assembly_mode = WOVEN_MODE;
		} else if (strcmp(argv[i], "-transposed") == 0) {
			assembly_mode = TRANSPOSED_MODE;
		} else if (strcmp(argv[i], "-flipped"   ) == 0) {
			assembly_mode = FLIPPED_MODE;
		} else {
			fprintf(stderr, "Unknown argument \"%s\".\n", argv[i-1]);
			print_usage();
		}
	}
}

void print_usage(void) {
	fprintf(stderr, "Usage: input file is standard input, output file is standard output.\n"
	                "Arguments:\n"
					"  -help        Display this information\n"
					"  -linear      Set assembly mode to linear\n"
					"  -woven       Set assembly mode to woven\n"
					"  -transposed  Set assembly mode to transposed\n"
					"  -flipped     Set assembly mode to flipped\n");
}

