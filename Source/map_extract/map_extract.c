#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map_extract.h"
#include "../compression/compression.h"

#pragma mark Constants

#define ATLAS_FILE "MAPHEAD.ext"  ///< File containing the levels atlas.
#define MAPS_FILE  "GAMEMAPS.ext" ///< File containing the levels.

#pragma mark Mappings

/**
 *  Converts a plane integer to its corresponding string.
 */
char *plane_to_string[MAP_PLANES] = {
	[MAP_ARCHITECTURE] = "Architecture",
	[MAP_OBJECTS     ] = "Objects"     ,
	[MAP_LOGIC       ] = "Logic"       ,
};

#pragma mark Functions

/**
 *  Loads the atlas data from the MAPHEADS file into the atlas buffer.
 *
 *  @return 0 on succes, otherwise an error code.
 */
int load_atlas();

/**
 *  Loads the header of a level from the GAMEMAPS file into a buffer and returns it.
 *
 *  @param gamemaps File pointer to an open GAMEMAPS file.
 *  @param episode  Episode of the level to load.
 *  @param level    Level within the episode.
 *
 *  @return Pointer to the allocated header structure.
 *
 *  The GAMEMAPS file needs to be open for reading and the atlas must have been loaded. If the funcion fails a `NULL` pointer is returned.
 */
struct level_header *load_header(FILE *gamemaps, uint episode, uint level);

/**
 *  Loads a specific map of a level from the GAMEMAPS file into a buffer and returns it.
 *
 *  @param gamemaps File pointer to an open GAMEMAPS file.
 *  @param header   Header of the map's level.
 *  @param map      Plane of the map to load.
 *
 *  @return Pointer to the allocated map buffer.
 *
 *  The GAMEMAPS file needs to be open for reading and the header must be valid. If the funcion fails a `NULL` pointer is returned.
 */
word *load_map(FILE *gamemaps, struct level_header *header, uint map);

/**
 *  Helper function, changes the template extension of a file name string.
 *
 *  @param file_name The file name to change.
 *  @param extension The new extension to apply.
 *
 *  This function assumes a fixed extension length of three characters.
 */
void change_extension(char *restrict file_name, const char *restrict extension);

#pragma mark Variables

struct level_atlas *atlas = NULL; ///< Variable holding the level atlas.

#pragma mark -
#pragma mark Implementation
int load_atlas() {
	if (atlas != NULL) {
		return 0;
	}
	
	char atlas_fname[] = ATLAS_FILE;
	change_extension(atlas_fname, extension);
	
	free(atlas);
	if ((atlas = malloc(sizeof(struct level_atlas))) == NULL) {
		return MALLOC_FAIL;
	}

	
	FILE *maphead;
	if ((maphead = fopen(atlas_fname, "rb")) == NULL) {
		fprintf(stderr, "Error: Could not open atlas file %s.\n", atlas_fname);
		return FILE_NOT_FOUND;
	}
	
	// Watch out, we can't just dump the stream into the atlas, the computer will add platform-specific padding (like making rlew_tag 4 bytes large)
	fread(&(atlas->rlew_tag), sizeof(word), 1, maphead);
	for (int i = 0; i < 100; ++i) {
		fread(&(atlas->header_offset[i]), sizeof(int32_t), 1, maphead);
	}
	
	fclose(maphead);
	maphead = NULL;
	//fwrite(atlas, sizeof(struct level_atlas), 1, stdout);
	return 0;
}

struct level_header *load_header(FILE *gamemaps, uint episode, uint level) {
	if (gamemaps == NULL || atlas == NULL) {
		return NULL;
	}
	
	uint index = (episode - 1) * EPISODE_LEVELS + level - 1;
	struct level_header *header = malloc(sizeof(struct level_header));
	
	fseek(gamemaps, atlas->header_offset[index], SEEK_SET);
	
	for (int i = 0; i < MAP_PLANES; ++i) {
		fread(&(header->map_offest[i]), sizeof(int32_t), 1, gamemaps);
	}
	for (int i = 0; i < MAP_PLANES; ++i) {
		fread(&(header->cc_length[i]), sizeof(word), 1, gamemaps);
	}
	fread(&(header->width ), sizeof(word), 1, gamemaps);
	fread(&(header->height), sizeof(word), 1, gamemaps);
	for (int i = 0; i < 16; ++i) {
		fread(&(header->name[i]), sizeof(char), 1, gamemaps);
	}
	//fread(header, 1, sizeof(struct level_header), gamemaps);
	
	return header;
}

word *load_map(FILE *gamemaps, struct level_header *header, uint map) {
	if (gamemaps == NULL || header == NULL) {
		return NULL;
	}
	
	fseek(gamemaps, header->map_offest[map], SEEK_SET);
	
	word *carmack_buffer = malloc(header->cc_length[map] * sizeof(word));
	if (carmack_buffer == NULL) {
		fprintf(stderr, "\t\tMemory Error: could not allocate memory to Carmack-expand map.\n");
		return NULL;
	}
	
	DEBUG_PRINT("Loading Carmack-compressed data...")
	fread(carmack_buffer, sizeof(word), header->cc_length[map] / 2, gamemaps); // the length is in bytes
	
	word rlew_compressed_length = *(carmack_buffer++) / 2; // stored in bytes, we want in words
	word *rlew_buffer = malloc(rlew_compressed_length * sizeof(word));
	if (rlew_buffer == NULL) {
		fprintf(stderr, "\t\tMemory Error: could not allocate memory to RLEW-expand map.\n");
		return NULL;
	}
	
	DEBUG_PRINT("Carmack-expanding from %i to %i words...", header->cc_length[map], rlew_compressed_length)
	carmack_expand(carmack_buffer, rlew_buffer, rlew_compressed_length);
	DEBUG_PRINT("Carmack-expanded...")
	
	--carmack_buffer; // move the Carmack buffer back one word to go back to the word we skipped earlier
	free(carmack_buffer);
	
	word uncompressed_length = header->width * header->height;
	word *map_buffer = malloc(uncompressed_length * sizeof(word));
	if (map_buffer == NULL) {
		fprintf(stderr, "\t\tMemory Error: could not allocate memory for uncompressed map.\n");
		return NULL;
	}
	
	DEBUG_PRINT("RLEW-expanding...")
	rlew_expand(rlew_buffer+1, map_buffer, uncompressed_length, atlas->rlew_tag);
	free(rlew_buffer);
	
	return map_buffer;
}

size_t extract_level_atlas(struct level_atlas **buffer) {
	if (load_atlas() != 0) {
		fprintf(stderr, "Could not load level atlas.\n");
		return 0;
	}
	free(*buffer);
	*buffer = atlas;
	return sizeof(atlas);
}

size_t extract_level_header(struct level_header **buffer, uint episode, uint level) {
	if (load_atlas() != 0) {
		fprintf(stderr, "Could not load level atlas.\n");
		return 0;
	}
	char maps_fname[] = MAPS_FILE;
	change_extension(maps_fname, extension);
	
	FILE *gamemaps = fopen(maps_fname, "rb");
	if (gamemaps == NULL) {
		fprintf(stderr, "Error: Could not open maps file %s.\n", maps_fname);
		return 0;
	}
	
	struct level_header *header = load_header(gamemaps, episode, level);
	fclose(gamemaps);
	if (header == NULL) {
		fprintf(stderr, "Error: Could not load header for level %i:%i.\n", episode, level);
		return 0;
	}
	
	free(*buffer);
	*buffer = header;
	return sizeof(struct level_header);
}

size_t extract_map(word **buffer, uint episode, uint level, uint map) {
	if (load_atlas() != 0) {
		fprintf(stderr, "Could not load level atlas.\n");
		return 0;
	}
	char maps_fname[] = MAPS_FILE;
	change_extension(maps_fname, extension);
	
	DEBUG_PRINT("Loading file...")
	FILE *gamemaps = fopen(maps_fname, "rb");
	if (gamemaps == NULL) {
		fprintf(stderr, "Error: Could not open maps file %s.\n", maps_fname);
		return FILE_NOT_FOUND;
	}
	
	DEBUG_PRINT("Loading header...")
	struct level_header *header = load_header(gamemaps, episode, level);
	if (header == NULL) {
		fprintf(stderr, "Error: Could not load header for level %i:%i.\n", episode, level);
		return LOAD_FAIL;
	}
	size_t size = header->width * header->height * sizeof(word);
	
	DEBUG_PRINT("Loading map...\n")
	word *result = load_map(gamemaps, header, map);
	if (result == NULL) {
		fprintf(stderr, "Error: Could not load map \"%s\" for level %i:%i.\n", plane_to_string[map], episode, level);
		return LOAD_FAIL;
	}
	free(header);
	
	free(*buffer);
	*buffer = result;
	return size;
}

void change_extension(char *restrict file_name, const char *restrict extension) {
	int n = (int)strlen(file_name); // strlen does not count the terminating `\0`.
	for (int i = 0; i < 3; ++i) {
		file_name[n - 3 + i] = extension[i];
	}
}
