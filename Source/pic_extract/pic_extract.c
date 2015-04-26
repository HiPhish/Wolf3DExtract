#include <stdlib.h>
#include <stdio.h>
#include "pic_extract.h"
#include "../compression/compression.h"
#include "../globals.h"


/*-[ CONSTANTS ]--------------------------------------------------------------*/

#define TREE_FILE  "VGADICT.ext"  ///< File containing the Huffman tree.
#define HEAD_FILE  "VGAHEAD.ext"  ///< File containing the picture offsets.
#define GRAPH_FILE "VGAGRAPH.ext" ///< File containing the pictures.

#define HUFFMAN_TREE_NODE_COUNT 255 ///< Number of nodes in the Huffman tree.


/*-[ TYPE DEFINITIONS ]-------------------------------------------------------*/

/** Structure holding the size of bitmap pictures. */
struct pic_size {
	int16_t width;  ///< Width of the pic
	int16_t height; ///< Height of the pic
};


/*-[ VARIABLE DEFINITIONS ]---------------------------------------------------*/

/** Tree structure for Huffman compression. */
struct huffman_node *huffman_tree;

/** Chunk indices of the first picture sorted by game version. */
int32_t pic_starts[GAME_VERSIONS] = {
	[WL1_I] = 3, ///< WL1 (placeholder)
	[WL3_I] = 3, ///< WL3 (placeholder)
	[WL6_I] = 3, ///< WL6
};

/** Chunk index of the last picture sorted by game version. */
int32_t pic_ends[GAME_VERSIONS] = {
	[WL1_I] = 134, ///< WL1 (placeholder)
	[WL3_I] = 134, ///< WL3 (placeholder)
	[WL6_I] = 134, ///< WL6
};

/** Number of pics per game version. */
int32_t pic_counts[GAME_VERSIONS] = {
	[WL1_I] = 132, ///< WL1 (placeholder)
	[WL3_I] = 132, ///< WL3 (placeholder)
	[WL6_I] = 132, ///< WL6
};

/** Number of chunks in the VGAGRAPH file sorted by game version. */
int32_t number_of_chunks[GAME_VERSIONS] = {
	[WL1_I] = 149, ///< WL1 (placeholder)
	[WL3_I] = 149, ///< WL3 (placeholder)
	[WL6_I] = 149, ///< WL6
};

uint             pic_start;     ///< Chunk index of the first pic.
uint             pic_end;       ///< Chunk index of the last pic.
uint             pic_count;     ///< Number of pictures used in the partincular version of the game.
struct pic_size *pic_table;     ///< Array of sizes for each pic.
int32_t         *graph_offsets; ///< Array of file offsets for each chunk in the VGAGRAPH file.


/*-[ FUNCTION DECLARATIONS ]--------------------------------------------------*/

/** Loads Huffman tree from a file and stores it in an array.
 *
 *  @param force  Whether to enforce reloading even if there is already a tree.
 *
 *  @return  Returns 0 on success, an error code otherwise.
 */
int load_huffman_tree(int force);

/** Load the picture offset into the offset array.
 *
 *  @param force  Whether to enforce reloading even if there is already a tree.
 *
 *  @return  Returns 0 on success, an error code otherwise.
 */
int load_pic_offsets(int force);

/** Load the picture table into the offset array.
 *
 *  @param force  Whether to enforce reloading even if there is already a tree.
 *
 *  @return  Returns 0 on success, an error code otherwise.
 */
int load_pic_table(int force);

/** Set various contstants based on the current game version. */
int set_constants(void);


/*-[ IMPLEMENTATIONS ]--------------------------------------------------------*/

int set_constants(void) {
	pic_start = pic_starts[current_game_version];
	pic_end   = pic_ends[  current_game_version];
	pic_count = pic_counts[current_game_version];
	return 0;
}

int load_huffman_tree(int force) {
	DEBUG_PRINT(1, "Loading Huffman tree... ")
	if (huffman_tree != NULL && !force) {
		return 0;
	}
	free(huffman_tree);

	char dict_fname[] = TREE_FILE;
	change_extension(dict_fname, extension);
	if ((huffman_tree = malloc(HUFFMAN_TREE_NODE_COUNT * sizeof(struct huffman_node))) == NULL) {
		return MALLOC_FAIL;
	}

	FILE *vgadict;
	if ((vgadict = fopen(dict_fname, "rb")) == NULL) {
		fprintf(stderr, "Error: Could not open VGA graphics tree file %s.\n", dict_fname);
		return FILE_NOT_FOUND;
	}

	// Read one word at a time into the array
	for (int i = 0; i < 255; ++i) {
		fread(&(huffman_tree[i].node_0), sizeof(word), 1, vgadict); 
		fread(&(huffman_tree[i].node_1), sizeof(word), 1, vgadict); 
	}

	fclose(vgadict);
	return 0;
}

int load_pic_offsets(int force) {
	DEBUG_PRINT(1, "Loading picture offsets... ")
	if (graph_offsets != NULL && !force){
		return 0;
	}
	free(graph_offsets);

	char head_fname[] = HEAD_FILE;
	change_extension(head_fname, extension);

	FILE *vgahead;
	if ((vgahead= fopen(head_fname, "rb")) == NULL) {
		fprintf(stderr, "Error: Could not load VGA head file %s.\n", head_fname);
		return FILE_NOT_FOUND;
	}

	if ((graph_offsets = malloc(number_of_chunks[current_game_version] * sizeof *graph_offsets)) == NULL) {
		fprintf(stderr, "Error: could not allocate memory to hold pic offsets.\n");
		return MALLOC_FAIL;
	}

	byte bytes[3] = {0x00, 0x00, 0x00};
	// read the file, three bytes make one number.
	for (int i = 0; i < number_of_chunks[current_game_version]; ++i) {
		fread(bytes, sizeof(byte), 3, vgahead);
		DEBUG_PRINT(2, "\n\tRead the following bytes: %x %x %x.", bytes[0], bytes[1], bytes[2])
		graph_offsets[i] = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16);
		if (graph_offsets[i] == 0x00FFFFFF) {
			graph_offsets[i] = -1;
		}
		DEBUG_PRINT(2, " Resulting number is %i.\n", graph_offsets[i])
	}

	fclose(vgahead);
	return 0;
}

int load_pic_table(int force) {
	DEBUG_PRINT(1, "Loading picture table... ")
	if (pic_table != NULL && !force){
		return 0;
	}
	free(pic_table);

	char graph_fname[] = GRAPH_FILE;
	change_extension(graph_fname, extension);

	FILE *vgagraph;
	if ((vgagraph = fopen(graph_fname, "rb")) == NULL) {
		fprintf(stderr, "Error: Could not load VGA graphics file \"%s\".\n", graph_fname);
		return FILE_NOT_FOUND;
	}
	DEBUG_PRINT(1, "\n\tLoaded VGAGRAPH file.")

	if ((pic_table = malloc(pic_count * sizeof *pic_table)) == NULL) {
		return MALLOC_FAIL;
	}

	// read and store the compressed pic table
	int32_t comressed_length = graph_offsets[1] - graph_offsets[0] - 4;
	int32_t expanded_length;
	fread(&expanded_length, sizeof(int32_t), 1, vgagraph);
	DEBUG_PRINT(1, "\n\tCompressed length of pic table is %i, expanded is %i.", comressed_length, expanded_length)

	byte *compressed_chunk = malloc (comressed_length * sizeof(byte));
	byte *expanded_chunk   = malloc (expanded_length  * sizeof(byte));

	if (compressed_chunk == NULL || expanded_chunk == NULL) {
		if (compressed_chunk) {free(compressed_chunk);}
		if (expanded_chunk  ) {free(expanded_chunk  );}

		fprintf(stderr, "Error allocating memory for decompressing picture table");
		return MALLOC_FAIL;
	}

	fread(compressed_chunk, sizeof *compressed_chunk, comressed_length, vgagraph);
	DEBUG_PRINT(1, "\n\tRead compressed chunk.")

	huffman_expand(compressed_chunk, expanded_chunk, expanded_length, huffman_tree);

	DEBUG_PRINT(1, "\n\tFreeing compressed chunk.")
	free(compressed_chunk);

	DEBUG_PRINT(1, "\n\tAssigning values.")
	for (int i = 0; i < pic_count; ++i) {
		pic_table[i].width  = expanded_chunk[4*i  ] | expanded_chunk[4*i+1];
		pic_table[i].height = expanded_chunk[4*i+2] | expanded_chunk[4*i+3];
		DEBUG_PRINT(2, "\n\t\tAssigned width %i and height %i.", pic_table[i].width, pic_table[i].height)
	}

	fclose(vgagraph);
	return 0;
}

size_t extract_pic_offsets(int32_t **buffer) {
	set_constants();
	if (load_huffman_tree(0) + load_pic_offsets(0) != 0) {
		fprintf(stderr, "Error loading picture offsets\n");
		return 0;
	}
	*buffer = graph_offsets;

	return pic_count * sizeof(int32_t);
}

size_t extract_pic_table(word **buffer) {
	set_constants();
	if (load_huffman_tree(0) + load_pic_offsets(0) + load_pic_table(0) != 0) {
		fprintf(stderr, "Error loading picture table.\n");
		return 0;
	}
	DEBUG_PRINT(1, "\n")
	if ((*buffer = malloc(pic_count * 2 * sizeof(word))) == NULL) {
		fprintf(stderr, "Error allocating memory for picture table");
		return 0;
	}

	*buffer = (word *)pic_table;
	return 2 * pic_count * sizeof(word);
}

size_t extract_pic(struct picture **buffer, uint magic_number){
	set_constants();
	if (magic_number < pic_start) {
		fprintf(stderr, "Error: %i not a valid magic number for pictures, must be in the range [%i, %i].\n", magic_number, pic_start, pic_end);
	}

	DEBUG_PRINT(1, "Beginning to extract picture %i.\n", magic_number)
	if (load_huffman_tree(0) + load_pic_offsets(0) + load_pic_table(0) != 0) {
		return 0;
	}
	DEBUG_PRINT(1, "Loaded everything.\n")

	if (graph_offsets[magic_number] == -1) {
		fprintf(stderr, "Magic Number is invalid, can't extract anything.\n");
		return 0;
	}

	char graph_fname[] = GRAPH_FILE;
	change_extension(graph_fname, extension);

	FILE *vgagraph;
	if ((vgagraph = fopen(graph_fname, "rb")) == NULL) {
		fprintf(stderr, "Error: Could not load VGA graphics file %s.\n", graph_fname);
		return FILE_NOT_FOUND;
	}

	uint next_index = magic_number + 1;
	while (graph_offsets[next_index] == -1 && next_index < pic_count) {
		++next_index;
	}
	if (graph_offsets[next_index] == -1) {
		return 0;
	}
	int compressed_length = graph_offsets[next_index] - graph_offsets[magic_number];
	
	DEBUG_PRINT(1, "Allocating compressed chunk of length %i.\n", compressed_length)
	int32_t *compressed_chunk = (int32_t *)malloc(compressed_length * sizeof(byte));
	if (compressed_chunk == NULL) {
		return 0;
	}
	
	fseek(vgagraph, graph_offsets[magic_number], SEEK_SET);
	fread(compressed_chunk, sizeof(byte) , compressed_length, vgagraph);

	int32_t expanded_length = *(compressed_chunk++);
	byte *expanded_chunk = malloc(expanded_length * sizeof(byte));
	if (expanded_chunk == NULL) {
		fprintf(stderr, "Error allocating memory for pixels.\n");
		return 0;
	}
	DEBUG_PRINT(1, "Allocated expanded chunk of length %i.\n", expanded_length)

	huffman_expand((byte *)compressed_chunk, expanded_chunk, expanded_length, huffman_tree);

	struct picture *pic = malloc(sizeof(struct picture));
	if (pic == NULL) {
		fprintf(stderr, "Error allocating memory for picture.\n");
		return 0;
	}

	pic->width   = pic_table[magic_number - pic_start].width;
	pic->height  = pic_table[magic_number - pic_start].height;
	pic->textels = (byte *)expanded_chunk;
	DEBUG_PRINT(1, "Allocated and assigned a picture of size %i x %i.\n", pic->width, pic->height)

	DEBUG_PRINT(1, "Freeing buffers\n")
	free(--compressed_chunk); // Remember that we had moved the pointer forward, so move it back
	fclose(vgagraph);
	//free(*buffer);
	*buffer = pic;
	DEBUG_PRINT(1, "Extracted picture.\n");
	return sizeof(struct picture);
}


