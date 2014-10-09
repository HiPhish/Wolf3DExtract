#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "map_extract/map_extract.h"
#include "pic_extract/pic_extract.h"

#pragma mark Constants

#define EXTRACT_LEVEL_ATLAS  "-la"  ///< Print the level atlas.
#define EXTRACT_LEVEL_HEADER "-lh"  ///< Print a level's header.
#define EXTRACT_MAP          "-lm"  ///< Print a level's map.
#define EXTRACT_PIC_OFFSETS  "-po"  ///< Print the pic offsets.
#define EXTRACT_PIC_TABLE    "-pt"  ///< Print the pic table.
#define EXTRACT_PIC          "-pic" ///< Print a bitmap picture (not a sprite or texture).
#define SPECIFY_EXTENSION    "-ext" ///< Specify a particular extension to use.
#define SET_DEBUG_LEVEL      "-dbg" ///< Set the debug level manually.

enum program_error {
	SUCCESS,
	INVALID_EPISODE,
	INVALID_LEVEL,
	INVALID_ARGS,
	INVALID_FILES,
	INVALID_MAP,
	
	PROGRAM_ERRORS
};

#pragma mark Mappings


#pragma mark Functions

/**
 *  Processed command-line arguments one after the other.
 *
 *  @param argc Amount of arguments.
 *  @param argv Array containing the arguments.
 */
void process_arguments(int argc, const char *argv[]);

/**
 *  Print usage instructions to a file.
 *
 *  @param file File to print to.
 *
 *  This function is usually called when the user passed no arguments or invalid ones. The recommended output file is the standard error.
 */
void print_usage(FILE *file);

/**
 *  Determine data file extension automatically.
 *
 *  @return 0 on success, otherwise non-zero.
 */
int determine_game_version();

/**
 *  Set the debug level for debug messages.
 *
 *  @param l String with the debug level, must represent an integer number.
 */
void set_debug_level(const char *l);

/**
 *  Set a specific file extension to use.
 *
 *  @param ext Extension to use
 */
void specify_extension(const char *ext);

/**
 *  Print the binary contents of the level atlas to the standard output.
 */
void print_level_atlas();

/**
 *  Print the binary contents of a level header to the standard output.
 *
 *  @param episode Episode of the level.
 *  @param level   Level within the episode.
 *
 *  Both `episode` and `level` are strings and get converted to integers by the function.
 */
void print_level_header(const char *restrict episode, const char *restrict level);

/**
 *  Print the binary contents of a level map to the standard output.
 *
 *  @param episode Episode of the level.
 *  @param level   Level within the episode.
 *  @param map     Map of the level, within the range 0 - 2.
 *
 *  All three `episode`, `level` and `map` are strings and get converted to integers by the function.
 */
void print_level_map(const char *restrict episode, const char *restrict level, const char *restrict map);

/** 
 *  Print the offsets of VGAGRAPH chunks to the standard output.
 *
 *  The offsets are given as signed 32-bit integers.
 */
void print_pic_offsets();

/**
 *  Print the picture size table to the standard output.
 *
 *  The table elements are pairs of signed 16-bit integers, where the first number of an element
 *  is the width and the second one is the height.
 */
void print_pic_table();

/**
 *  Print a bitmap picture (not a sprite or texture) to the standard output.
 *
 *  @param magic_number Magic number of the image.
 *
 *  The magic number is dependent on the version of the game.
 */
void print_picture(const char *restrict magic_number);

#pragma mark -

int main(int argc, const char *argv[]) {
	determine_game_version();
	process_arguments(argc, argv);
	
    return 0;
}

#pragma mark -

int determine_game_version() {
	current_game_version = WL6_I; // <-- Placeholder for now
	strncpy(extension, extensions[current_game_version], 3);
	return 0;
}

void set_debug_level(const char *l) {
	debug_level = (uint)strtol(l, NULL, 10);
}

void process_arguments(int argc, const char *argv[]) {
	if (argc == 1) {
		print_usage(stderr);
		return;
	}
	int i = 1; // Skip argv[0] (program name).
	while (i < argc) {
		if (       strcmp(argv[i], EXTRACT_LEVEL_ATLAS)  == 0) {
			print_level_atlas();
		} else if (strcmp(argv[i], EXTRACT_LEVEL_HEADER) == 0) {
			print_level_header(argv[i+1], argv[i+2]);
			i += 2;
		} else if (strcmp(argv[i], EXTRACT_MAP)          == 0) {
			print_level_map(argv[i+1], argv[i+2], argv[i+3]);
			i += 3;
		} else if (strcmp(argv[i], EXTRACT_PIC_TABLE)    == 0) {
			print_pic_table();
		} else if (strcmp(argv[i], EXTRACT_PIC_OFFSETS)  == 0) {
			print_pic_offsets();
		} else if (strcmp(argv[i], EXTRACT_PIC)          == 0) {
			print_picture(argv[++i]);
		} else if (strcmp(argv[i], SET_DEBUG_LEVEL)      == 0) {
			set_debug_level(argv[++i]);
		} else if (strcmp(argv[i], SPECIFY_EXTENSION)    == 0) {
			specify_extension(argv[++i]);
		} else {
			fprintf(stderr, "Unknown argument %s.\n", argv[i]);
			print_usage(stderr);
			exit(INVALID_ARGS);
		}
		++i;
	}
}

void print_usage(FILE *file) {
	fprintf(file, "Usage: Call from the same directory where your data files are located and pass the following arguments\n"
				  "\t"SPECIFY_EXTENSION    " WLX   " "Set the extension of the data files to the argument WLX\n"
				  "\t"SET_DEBUG_LEVEL      " n     " "Set the level of debug messages (default 0, no messages)\n"
			      "\t"EXTRACT_LEVEL_ATLAS  "        " "Extract the atlas of the levels\n"
			      "\t"EXTRACT_LEVEL_HEADER "  e l   " "Extract the header data for the specified level (level and episode given as numbers)\n"
			      "\t"EXTRACT_MAP          "  e l m " "Extract the specified map for the specified level (map in the range 0 - 2)\n"
			      "\t"EXTRACT_PIC_OFFSETS  "        " "Extract the picture offsets\n"
			      "\t"EXTRACT_PIC_TABLE    "        " "Extract the picture table\n"
			      "\t"EXTRACT_PIC          " m     " "Extract the picture with the specified magic number\n"
			      "The output is printed to the standard output, so you'll want to redirect it into another file or pipe it into another program.\n"
				  "Arguments are processed in the order they are give, so if for exapmle you want to specify the extension you have to do it before trying to extract an asset.\n"
			);
}

void specify_extension(const char *ext) {
	for (int i = 0; i < 3; ++i) {
		extension[i] = ext[i];
	}
}

void print_level_atlas() {
	struct level_atlas *result = NULL;
	
	size_t written_bytes = extract_level_atlas(&result);
	
	written_bytes *= fwrite(&(result->rlew_tag), sizeof(word), 1, stdout);
	for (int i = 0; i < MAX_LEVELS; ++i) {
		written_bytes *= fwrite(&(result->header_offset[i]), sizeof(int32_t), 1, stdout);
	}
	
	if (written_bytes == 0) {
		fprintf(stderr, "\tError writing level atlas.\n");
	}
}

void print_level_header(const char *restrict episode, const char *restrict level) {
	struct level_header *result = NULL;
	int e = (uint)strtol(episode, NULL, 10);
	int l = (uint)strtol(level  , NULL, 10);
	
	size_t written_bytes = extract_level_header(&result, e, l);
	
	for (int i = 0; i < MAP_PLANES; ++i) {
		written_bytes *= fwrite(&(result->map_offest[i]), sizeof(int32_t), 1, stdout);
	}
	for (int i = 0; i < MAP_PLANES; ++i) {
		written_bytes *= fwrite(&(result->cc_length[i]), sizeof(word), 1, stdout);
	}
	written_bytes *= fwrite(&(result->width ), sizeof(word), 1, stdout);
	written_bytes *= fwrite(&(result->height), sizeof(word), 1, stdout);
	for (int i = 0; i < 16; ++i) {
		written_bytes *= fwrite(&(result->name[i]), sizeof(char), 1, stdout);
	}
	
	if (written_bytes == 0) {
		fprintf(stderr, "\tError writing level header for episode %i level %i.\n", e, l);
	} else {
		DEBUG_PRINT(1, "Wrote level header for episode % i, level %i.\n", e, l);
	}
}

void print_level_map(const char *restrict episode, const char *restrict level, const char *restrict map) {
	word *result = NULL;
	//struct level_header *header = NULL;
	int e = (uint)strtol(episode, NULL, 10);
	int l = (uint)strtol(level  , NULL, 10);
	int m = (uint)strtol(map    , NULL, 10);
	
	//extract_level_header(&header, e, l);
	size_t written_bytes = extract_map(&result, e, l, m);
	
	//for (word i = 0; i < header->height * header->width; ++i) {
	for (word i = 0; i < 64*64; ++i) {
		written_bytes *= fwrite(&result[i], sizeof(word), 1, stdout);
	}
	
	if (written_bytes == 0) {
		fprintf(stderr, "\tError writing level map %i for episode %i level %i.\n", m, e, l);
	} else {
		DEBUG_PRINT(1, "Wrote map %i of episode % i, level %i.\n", m, e, l);
	}
}

void print_pic_offsets() {
	int32_t *result;
	size_t length = extract_pic_offsets(&result) / sizeof(int32_t);
	fwrite(result, sizeof(int32_t), length, stdout);
}

void print_pic_table() {
	word *result;
	size_t length = extract_pic_table(&result) / sizeof(word);
	fwrite(result, sizeof *result, length, stdout);
}

void print_picture(const char *restrict magic_number) {
	int i = (uint)strtol(magic_number, NULL, 10);
	struct picture *result = NULL;
	size_t written_bytes = extract_pic(&result, i);

	uint number_of_bytes = result->width * result->height;
	written_bytes *= fwrite(&(result->width), sizeof(word), 1, stdout) * fwrite(&(result->height), sizeof(word), 1, stdout);
	written_bytes *= fwrite(result->textels, sizeof(byte), number_of_bytes, stdout);


	if (written_bytes == 0) {
		fprintf(stderr, "\tError writing picture %i.\n", i);
	} else {
		DEBUG_PRINT(1, "Wrote picture %i.\n", i);
	}
}

