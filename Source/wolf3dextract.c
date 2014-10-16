#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "map_extract/map_extract.h"
#include "pic_extract/pic_extract.h"
#include "sprite_extract/sprite_extract.h"
#include "audio_extract/audio_extract.h"

#pragma mark Constants

#define EXTRACT_LEVEL_ATLAS  "-la"  ///< Print the level atlas.
#define EXTRACT_LEVEL_HEADER "-lh"  ///< Print a level's header.
#define EXTRACT_MAP          "-lm"  ///< Print a level's map.
#define EXTRACT_PIC_OFFSETS  "-po"  ///< Print the pic offsets.
#define EXTRACT_PIC_TABLE    "-pt"  ///< Print the pic table.
#define EXTRACT_PIC          "-pic" ///< Print a bitmap picture (not a sprite or texture).
#define EXTRACT_TEXTURE      "-tex" ///< Print a texture.
#define EXTRACT_SPRITE       "-spr" ///< Print a sprite.
#define EXTRACT_SOUND        "-snd" ///< Print a sound effect.
#define EXTRACT_MUSIC        "-mus" ///< Print a music track.
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

/** Compare two strings and evaluate to `1` if they are equal, `0` otherwise. */
#define STRING_EQUAL(s1, s2) (strcmp(s1, s2)  == 0) ? 1 : 0

/** Compare the current argument with a given string. */
#define ARG_EQUAL(arg) STRING_EQUAL(argv[i], arg)

#pragma mark Functions

/**
 *  Processed command-line arguments one after the other.
 *
 *  @param argc Amount of arguments.
 *  @param argv Array of strings containing the arguments.
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
int determine_game_version(void);

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
void print_level_atlas(void);

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
void print_pic_offsets(void);

/**
 *  Print the picture size table to the standard output.
 *
 *  The table elements are pairs of signed 16-bit integers, where the first number of an element
 *  is the width and the second one is the height.
 */
void print_pic_table(void);

/**
 *  Print a bitmap picture (not a sprite or texture) to the standard output.
 *
 *  @param magic_number Magic number of the image.
 *
 *  The magic number is dependent on the version of the game.
 */
void print_picture(const char *restrict magic_number);

/**
 *  Print a texture to the standard output.
 *
 *  @param magic_number Magic number of the texture.
 *
 *  The magic number is dependent on the version of the game.
 */
void print_texture(const char *restrict magic_number);

/**
 *  Print a  sprite to the standard output.
 *
 *  @param magic_number Magic number of the sprite.
 *
 *  The magic number is dependent on the version of the game.
 */
void print_sprite(const char *restrict magic_number);

/**
 * Print a sound effect to the standard output.
 *
 * @param magic_number Magic number of the sound effect.
 * @param format       Format of the sound effect (`pc` or `adlib`)
 */
void print_sound(const char *restrict magic_number, const char *restrict format);

/**
 * Print a music track to the standard output.
 *
 * @param magic_number Magic number of the music track.
 */
void print_music(const char *restrict magic_number);

#pragma mark -

int main(int argc, const char *argv[]) {
	determine_game_version();
	process_arguments(argc, argv);
	
    return 0;
}

#pragma mark -

int determine_game_version(void) {
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
		if (       ARG_EQUAL(EXTRACT_LEVEL_ATLAS) ) {
			print_level_atlas();
		} else if (ARG_EQUAL(EXTRACT_LEVEL_HEADER)) {
			print_level_header(argv[i+1], argv[i+2]);
			i += 2;
		} else if (ARG_EQUAL(EXTRACT_MAP)         ) {
			print_level_map(argv[i+1], argv[i+2], argv[i+3]);
			i += 3;
		} else if (ARG_EQUAL(EXTRACT_PIC_TABLE)   ) {
			print_pic_table();
		} else if (ARG_EQUAL(EXTRACT_PIC_OFFSETS) ) {
			print_pic_offsets();
		} else if (ARG_EQUAL(EXTRACT_PIC)         ) {
			print_picture(argv[++i]);
		} else if (ARG_EQUAL(EXTRACT_TEXTURE)     ) {
			print_texture(argv[++i]);
		} else if (ARG_EQUAL(EXTRACT_SPRITE)      ) {
			print_sprite(argv[++i]);
		} else if (ARG_EQUAL(EXTRACT_SOUND)       ) {
			print_sound(argv[i+1], argv[i+2]);
			i += 2;
		} else if (ARG_EQUAL(EXTRACT_MUSIC)       ) {
			print_music(argv[++i]);
		} else if (ARG_EQUAL(SET_DEBUG_LEVEL)     ) {
			set_debug_level(argv[++i]);
		} else if (ARG_EQUAL(SPECIFY_EXTENSION)   ) {
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
				  "  "SPECIFY_EXTENSION    " WLX   "  "Set the extension of the data files to the argument WLX\n"
				  "  "SET_DEBUG_LEVEL      " n     "  "Set the level of debug messages (default 0, no messages)\n"
			      "  "EXTRACT_LEVEL_ATLAS  "        " "Extract the atlas of the levels\n"
			      "  "EXTRACT_LEVEL_HEADER "  e l   " "Extract the header data for the specified level (level and episode given as numbers)\n"
			      "  "EXTRACT_MAP          "  e l m " "Extract the specified map for the specified level (map in the range 0 - 2)\n"
			      "  "EXTRACT_PIC_OFFSETS  "        " "Extract the picture offsets\n"
			      "  "EXTRACT_PIC_TABLE    "        " "Extract the picture table\n"
			      "  "EXTRACT_PIC          " m     "  "Extract the picture with the specified magic number\n"
			      "  "EXTRACT_TEXTURE      " m     "  "Extract the texture with the specified magic number\n"
			      "  "EXTRACT_SPRITE       " m     "  "Extract the sprite with the specified magic number\n"
			      "  "EXTRACT_SOUND        " m f   "  "Extract the sound effect with the specified magic number and format\n"
			      "  "EXTRACT_MUSIC        " m     "  "Extract the music track with the specified magic number\n"
			      "The output is printed to the standard output, so you'll want to redirect it into another file or pipe it into another program.\n"
				  "Arguments are processed in the order they are give, so if for exapmle you want to specify the extension\n"
 				  "you have to do it before trying to extract an asset.\n"
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
	free(result);
}

void print_level_header(const char *restrict episode, const char *restrict level) {
	struct level_header *result = NULL;
	uint e = (uint)strtol(episode, NULL, 10);
	uint l = (uint)strtol(level  , NULL, 10);
	
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
	free(result);
}

void print_level_map(const char *restrict episode, const char *restrict level, const char *restrict map) {
	word *r = NULL;
	uint  e = (uint)strtol(episode, NULL, 10);
	uint  l = (uint)strtol(level  , NULL, 10);
	uint  m = (uint)strtol(map    , NULL, 10);
	
	size_t written_bytes = extract_map(&r, e, l, m);
	
	for (word i = 0; i < 64*64; ++i) {
		written_bytes *= fwrite(&r[i], sizeof(word), 1, stdout);
	}
	
	if (written_bytes == 0) {
		fprintf(stderr, "\tError writing level map %i for episode %i level %i.\n", m, e, l);
	} else {
		DEBUG_PRINT(1, "Wrote map %i of episode % i, level %i.\n", m, e, l);
	}
	free(r);
}

void print_pic_offsets(void) {
	int32_t *r;
	size_t   l = extract_pic_offsets(&r) / sizeof(int32_t);
	fwrite(r, sizeof(int32_t), l, stdout);
	free(r);
}

void print_pic_table(void) {
	word   *r;
	size_t  l = extract_pic_table(&r) / sizeof(word);
	fwrite(r, sizeof *r, l, stdout);
	free(r);
}

void print_picture(const char *restrict magic_number) {
	uint i = (uint)strtol(magic_number, NULL, 10);
	struct picture *result = NULL;
	size_t written_bytes = extract_pic(&result, i);

	int number_of_bytes = result->width * result->height;
	written_bytes *= fwrite(&(result->width), sizeof(word), 1, stdout) * fwrite(&(result->height), sizeof(word), 1, stdout);
	written_bytes *= fwrite(result->textels, sizeof(byte), (size_t)number_of_bytes, stdout);

	if (written_bytes == 0) {
		fprintf(stderr, "\tError writing picture %i.\n", i);
	} else {
		DEBUG_PRINT(1, "Wrote picture %i.\n", i);
	}
	free(result);
}

void print_texture(const char *restrict magic_number) {
	uint  i    = (uint)strtol(magic_number, NULL, 10);
	byte  s[4] = {0x40, 0x00, 0x40, 0x00}; // The size of the texture in words (64=0x40)
	byte *r    = NULL;
	size_t written_bytes = extract_texture(&r, i);

	written_bytes *= fwrite(s  , sizeof(byte),     4, stdout);
	written_bytes *= fwrite(r, sizeof(byte), 64*64, stdout);

	if (written_bytes == 0) {
		fprintf(stderr, "\tError writing picture %i.\n", i);
	}
	free(r);
}

void print_sprite(const char *restrict magic_number) {
	uint  i    = (uint)strtol(magic_number, NULL, 10);
	byte  s[4] = {0x40, 0x00, 0x40, 0x00}; // The size of the texture in words (64=0x40)
	byte *r    = NULL;

	size_t written_bytes = extract_sprite(&r, i);

	written_bytes *= fwrite(s  , sizeof(byte),     4, stdout);
	written_bytes *= fwrite(r, sizeof(byte), 64*64, stdout);

	if (written_bytes == 0) {
		fprintf(stderr, "\tError writing picture %i.\n", i);
	}
	free(r);
}

void print_sound(const char *restrict magic_number, const char *restrict format) {
	uint          i = (uint)strtol(magic_number, NULL, 10);
	sound_format  f;
	size_t        l;
	byte         *r;

	if (strncmp(format, "p", 1) == 0) {
		f = PC_SPEAKER;
	} else if (strncmp(format, "a", 1) == 0) {
		f = ADLIB_SOUND;
	} else if (strncmp(format, "d", 1) == 0) {
		f = DIGI_SOUND;
	} else {
		fprintf(stderr, "Error: unknown format \"%s\".\n", format);
		return;
	}
	
	if ((l = extract_sound(&r, i, f)) == 0) {
		return;
	}
	
	// Digitised sound is raw PCM data, so we need to prepend the file size.
	if (f == DIGI_SOUND) {
		word s = (word)l;
		fwrite(&s, sizeof(word), 1, stdout);
	}
	fwrite(r, sizeof(byte), l, stdout);
	free(r);
}

void print_music(const char *restrict magic_number) {
	uint    i = (uint)strtol(magic_number, NULL, 10);
	size_t  l;
	byte   *r;
	if ((l = extract_music(&r, i)) == 0) {
		return;
	}
	fwrite(r, sizeof(byte), l, stdout);
	free(r);
}

