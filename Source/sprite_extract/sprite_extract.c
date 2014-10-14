#include <stdio.h>
#include <string.h>
#include "sprite_extract.h"
#include "../globals.h"

#define VSWAP_FILE   "VSWAP.ext" ///< File containing the sprites.
#define TRANSPARENCY 0xFF        ///< Colour for transparent texels.

// Error codes
#define SE_FILE_NOT_FOUND 1 ///< Error opening file.
#define SE_MALLOC_FAIL    2 ///< Error trying to allocate memory.

/** Structure holding the location and position of a chunk. */
struct vswap_chunk_header {
	uint32_t offset; ///< Offset of the chunk relative to the beginning of the file.
	word     length; ///< Length of the chunk in bytes ???
};

static int16_t number_of_chunks; ///< Total number of chunks in the file.
static int16_t sprite_start;     ///< Offset to the first sprite chunk in the file.
static int16_t sound_start;      ///< Offset to the first sound chunk in the file.

static struct vswap_chunk_header *headers; // Array of chunk headers.

/**
 * Loads the VSWAP file header and set variables.
 *
 * @return 0 if everything went right, otherwise an error code.
 */
static int load_vswap_header(void);

/** Open the VSWAP file and keep it open for use. */
#define LOAD_VSWAP_HEADER                                                    \
	char vswap_fname[] = VSWAP_FILE;                                         \
	change_extension(vswap_fname, extension);                                \
	FILE *vswap = fopen(vswap_fname, "rb");                                  \
	if (vswap == NULL) {                                                     \
		fprintf(stderr, "Error: could not load file \"%s\".\n", vswap_fname);\
		return SE_FILE_NOT_FOUND;                                            \
	}                                                                        \

/** Prepare the VSWAP file for extraction. */
#define PREPARE_VSWAP_FILE         \
	if (load_vswap_header() != 0) {\
		return 0;                  \
	}                              \
	LOAD_VSWAP_HEADER              \

static int load_vswap_header(void) {
	LOAD_VSWAP_HEADER

	// read the number of chunks and offset
	fread(&number_of_chunks, sizeof number_of_chunks, 1, vswap);
	fread(&sprite_start, sizeof sprite_start, 1, vswap);
	fread(&sound_start, sizeof sound_start, 1, vswap);

	// read the chunk headers
	free(headers);
	if ((headers = malloc(number_of_chunks * sizeof(struct vswap_chunk_header))) == NULL) {
		fprintf(stderr, "Error: Could not allocate memory to hold VSWAP chunk headers.\n");
		return SE_MALLOC_FAIL;
	}
	for (int i = 0; i < number_of_chunks; ++i) {
		fread(&headers[i].offset, sizeof(uint32_t), 1, vswap);
	}
	for (int i = 0; i < number_of_chunks; ++i) {
		fread(&headers[i].length, sizeof(word), 1, vswap);
	}
	
	fclose(vswap);
	vswap = NULL;
	DEBUG_PRINT(1, "Loaded VSWAP header.\n");
	return 0;
}

size_t extract_texture(byte **buffer, uint magic_number) {
	PREPARE_VSWAP_FILE
	if (magic_number >= sprite_start) {
		fprintf(stderr, "Error: invalid magic number for textures, must be within range [0, %i).\n", sprite_start);
		fclose(vswap);
		return 0;
	}
	
	if ((*buffer = malloc(headers[magic_number].length * sizeof(byte))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory to hold compressed sprite.\n");
		return 0;
	}
	fseek(vswap, headers[magic_number].offset, SEEK_SET);
	fread(*buffer, sizeof(byte), headers[magic_number].length, vswap);
	fclose(vswap);

	return 64 * 64 * sizeof(byte);
}

size_t extract_sprite(byte **buffer, uint magic_number) {
	PREPARE_VSWAP_FILE
	magic_number += sprite_start; // always add the constant sprite offset
	if (magic_number < sprite_start || magic_number >= sound_start) {
		fprintf(stderr, "Error: invalid magic number for sprites, must be within range [0, %i).\n", sound_start - sprite_start);
		fclose(vswap);
		return 0;
	}
	
	byte *compressed_chunk;
	word  first_column, last_column; // first and last column of the sprite with non-transparent texels (left->right)
	word *column_offsets;            // sequence of offsets to the column instructions.
	
	// Read the compressed chunk from the file and close it
	if ((compressed_chunk = malloc(headers[magic_number].length * sizeof(byte))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory to hold compressed sprite.\n");
		return 0;
	}
	fseek(vswap, headers[magic_number].offset, SEEK_SET);
	fread(compressed_chunk, sizeof(byte), headers[magic_number].length, vswap);
	fclose(vswap);
	DEBUG_PRINT(1, "Read compressed chunk of size %i bytes.\n", headers[magic_number].length)

	// fill the variables.
	first_column = (word)compressed_chunk[0] | (word)(compressed_chunk[1])<<8;
	last_column  = (word)compressed_chunk[2] | (word)(compressed_chunk[3])<<8;
	DEBUG_PRINT(1, "First column: %i, last column: %i.\n", first_column, last_column);

	if ((column_offsets = malloc((last_column - first_column + 1) * sizeof(word))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory for column instruction offsets.\n");
		return 0;
	}
	for (int i = 0; i <= last_column - first_column; ++i) {
		column_offsets[i] = (word)compressed_chunk[4+2*i] | (word)(compressed_chunk[4+2*i+1])<<8;
	}
	DEBUG_PRINT(1, "Read column instruction offsets.\n");

	free(*buffer);
	if ((*buffer = malloc(64 * 64 * sizeof(byte))) == NULL) {
			fprintf(stderr, "Error: could not allocate memory to hold decompressed sprite.");
			*buffer = NULL;
			return 0;
	}
	memset(*buffer, TRANSPARENCY, 64*64); // fill all the holes
	DEBUG_PRINT(1, "Set up the buffer.\n");

	//int i = (last_column - first_column + 1 + 2) * sizeof(word); // two words for first and last colum and the offsets
	word *column_offset_reader = column_offsets; // read-head that will traverse the column offsets

	for (word column = first_column; column <= last_column; ++column) {
		DEBUG_PRINT(2, "Drawing column %i...\n", column);
		word *drawing_instructions = (word *)(compressed_chunk + *column_offset_reader);
		uint idx = 0;
		while (drawing_instructions[idx] != 0) {
			for (int row = drawing_instructions[idx+2] / 2; row < drawing_instructions[idx] / 2; ++row) {
				DEBUG_PRINT(2, "\tDrawing row %i...\n", row);
				(*buffer)[column + (63 - row) * 64] = compressed_chunk[drawing_instructions[idx+1]+ row];
				//(*buffer)[column + (63 - row) * 64] = compressed_chunk[i];
				DEBUG_PRINT(2, "Drew at position %i + (63 - %i) * 64 = %i.\n", column, row, column + (63-row)*64)
				//++i;
			}
			idx += 3;
		}
		++column_offset_reader; // advance the read-head
	}

	free(compressed_chunk);
	return 64 * 64 * sizeof(byte);
}

