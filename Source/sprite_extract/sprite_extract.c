#include <stdio.h>
#include "sprite_extract.h"
#include "../globals.h"

#define VSWAP_FILE "VSWAP.ext" ///< File containing the sprites.

/** Structure holding the location and position of a chunk. */
struct vswap_chunk_header {
	uint32_t offset; ///< Offset of the chunk relative to ???
	word     length; ///< Length of the chunk in bytes ???
};

static int16_t number_of_chunks; ///< Total number of chunks in the file.
static int16_t sprite_start;     ///< Offset to the sprite chunk in the file.

static struct vswap_chunk_header *headers; // Array of chunk headers.

/**
 * Loads the VSWAP file header and set variables.
 *
 * @return 0 if everything went right, otherwise an error code.
 */
static int load_vswap_header(void);

/** Open the vswap file and keep it open for use. */
#define PREPARE_VSWAP_FILE \
	char vswap_fname[] = VSWAP_FILE;\
	change_extension(vswap_fname, extension);\
	FILE *vswap = fopen(vswap_fname, "rb");\
	if (vswap == NULL) {\
		fprintf(stderr, "Error: could not load file \"%s\".\n", vswap_fname);\
		return SE_FILE_NOT_FOUND;\
	}\

static int load_vswap_header(void) {
	PREPARE_VSWAP_FILE

	// read the number of chunks and offset
	fread(&number_of_chunks, sizeof number_of_chunks, 1, vswap);
	fread(&sprite_start, sizeof sprite_start, 1, vswap);
	fseek(vswap, 1 * sizeof(int16_t), SEEK_CUR); // skip over the sound start

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
	return 0;
}

size_t extract_sprite(byte **buffer, uint index) {
	load_vswap_header();
	PREPARE_VSWAP_FILE
	
	word  first_column, last_column; // first and last column of the sprite with non-transparent pixels (left->right)
	word *column_offsets;
	byte *compressed_chunk;
	
	if ((compressed_chunk = malloc(headers[index].length * sizeof(byte))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory to hold compressed sprite.\n");
		return SE_MALLOC_FAIL;
	}
	fseek(vswap, headers[index].offset, SEEK_SET);
	fread(compressed_chunk, sizeof(byte), headers[index].length, vswap);
	fclose(vswap);

	// fill the variables.
	first_column = (word)compressed_chunk[0] | (word)(compressed_chunk[1])<<8;
	last_column  = (word)compressed_chunk[2] | (word)(compressed_chunk[3])<<8;
	for (int i = 0; i <= last_column - first_column; ++i) {
		column_offsets[i] = (word)compressed_chunk[4+2*i] | (word)(compressed_chunk[4+2*i+1])<<8;
	}

	free(*buffer);
	if ((*buffer = malloc(64 * 64 * sizeof(byte))) == NULL) {
			fprintf(stderr, "Error: could not allocate memory to hold decompressed sprite.");
			*buffer = NULL;
			return SE_MALLOC_FAIL;
	}

	int i = (last_column - first_column + 1 + 2) * sizeof(word); // two words for first and last colum and the offsets
	word *column_offset_reader = column_offsets; // read-head that will traverse the column offsets

	for (int column = first_column; column <= last_column; ++column) {
		word current_column_offset = *column_offset_reader;
		word *drawing_instructions = (word *)(compressed_chunk + current_column_offset);
		uint idx = 0;
		while (drawing_instructions[idx] != 0) {
			for (int row = drawing_instructions[idx + 2]; row < drawing_instructions[idx] / 2; ++row) {
				*buffer[column + (63 - row) * 64] = compressed_chunk[i];
				++i;
			}
			idx += 3;
		}
		++column_offset_reader; // advance the read-head
	}

	free(compressed_chunk);
	return 64 * 64 * sizeof(byte);
}

#undef PREPARE_VSWAP_FILE

