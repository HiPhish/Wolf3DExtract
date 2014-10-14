#include <stdio.h>
#include "audio_extract.h"

// Error codes
#define AE_MALLOC_FAIL    1 ///< Error opening file.
#define AE_FILE_NOT_FOUND 2 ///< Error trying to allocate memory.

#define AUDIOHED_FILE "AUDIOHED.ext" ///< File containing the audio offsets.
#define AUDIOT_FILE   "AUDIOT.ext"

/** Number of sound effects per type (PC speaker, Adlib, digitized). */
uint32_t number_of_sounds[GAME_VERSIONS] = {
	[WL1_I] = 87, ///< WL1 (placeholder)
	[WL3_I] = 87, ///< WL3 (placeholder)
	[WL6_I] = 87, ///< WL6
};

/** Number of sound effects per type (PC speaker, Adlib, digitized). */
uint32_t number_of_music[GAME_VERSIONS] = {
	[WL1_I] = 27, ///< WL1 (placeholder)
	[WL3_I] = 27, ///< WL3 (placeholder)
	[WL6_I] = 27, ///< WL6
};

char *format_to_string[SOUND_FORMATS] = {
	[PC_SPEAKER ] = "PC speaker",
	[ADLIB_SOUND] = "AdLib"
};

// Starting offsets for the individual sound types
#define START_PC_SOUND    (0*number_of_sounds[current_game_version]) ///< Offset to the first PC speaker chunk.
#define START_ADLIB_SOUND (1*number_of_sounds[current_game_version]) ///< Offset to the first Adlib chunk.
#define START_DIGI_SOUND  (2*number_of_sounds[current_game_version]) ///< Offset to the first digitized chunk.
#define START_MUSIC       (3*number_of_sounds[current_game_version]) ///< Offset to the first music chunk.

/** Total number of chunks to read, plus one more past the end. */
#define NUMBER_OF_CHUNKS (3*number_of_sounds[current_game_version]+number_of_music[current_game_version]+1)

/** Sequence of audion chunk offsets. */
uint32_t *chunk_offsets;

/** Load the offsets of the audio chunks into the offset buffer. */
int load_chunk_offsets(void);

/**
 * Loads a PC speaker sound effect into a buffer.
 *
 * @param buffer Pointer to a non-allocated byte-sequence to store the data into.
 * @param offset Offset of the chunk in the *AUDIOT* file.
 * @param length Length of the chunk in the *AUDIOT* file.
 *
 * @return Length of the chunk, or 0 if an error occured.
 */
size_t load_pcs_sound(byte **buffer, int32_t magic_number, int32_t length);

/**
 * Loads an AdLib sound effect into a buffer.
 *
 * @param buffer Pointer to a non-allocated byte-sequence to store the data into.
 * @param offset Offset of the chunk in the *AUDIOT* file.
 * @param length Length of the chunk in the *AUDIOT* file.
 *
 * @return Length of the chunk, or 0 if an error occured.
 */
size_t load_adlib_sound(byte **buffer, int32_t magic_number, int32_t length);

/**
 * Loads a music track into a buffer.
 *
 * @param buffer Pointer to a non-allocated byte-sequence to store the data into.
 * @param offset Offset of the chunk in the *AUDIOT* file.
 * @param length Length of the chunk in the *AUDIOT* file.
 *
 * @return Length of the chunk, or 0 if an error occured.
 */
size_t load_music_track(byte **buffer, int32_t magic_number, int32_t length);

int load_chunk_offsets(void) {
	if ((chunk_offsets = malloc(NUMBER_OF_CHUNKS * sizeof(uint32_t))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory to hold audio offsets.\n");
		return AE_MALLOC_FAIL;
	}

	char audiohed_fname[] = AUDIOHED_FILE;
	change_extension(audiohed_fname, extension);
	FILE *audiohed = fopen(audiohed_fname, "rb");
	if (audiohed == NULL) {
		fprintf(stderr, "Error: could not open file \"%s\".\n", audiohed_fname);
		free(chunk_offsets);
		return AE_FILE_NOT_FOUND;
	}
	fread(chunk_offsets, sizeof(uint32_t), NUMBER_OF_CHUNKS, audiohed); 
	fclose(audiohed);

	DEBUG_PRINT(1, "Loaded chunk offsets.\n");
	return 0;
}

size_t load_pcs_sound(byte **buffer, int32_t magic_number, int32_t length) {
	char audiot_fname[] = AUDIOT_FILE;
	change_extension(audiot_fname, extension);
	FILE *audiot = fopen(audiot_fname, "rb");
	if (audiot == NULL) {
		fprintf(stderr, "Error: could not open the audio file \"%s\".\n", audiot_fname);
		return 0;
	}
	DEBUG_PRINT(1, "Opened AUDIOT file.\n")

	if ((*buffer = malloc(length * sizeof(byte))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory for sound effect.\n");
	}
	DEBUG_PRINT(1, "Allocated memory for sound chunk.\n")
	fseek(audiot, chunk_offsets[magic_number], SEEK_SET);
	fread(*buffer, sizeof(byte), length, audiot);
	fclose(audiot);

	DEBUG_PRINT(1, "Read shound chunk.\n")
	return length;
}

size_t load_adlib_sound(byte **buffer, int32_t magic_number, int32_t length) {
	// There is no real difference between PC speaker and AdLib in regards to extraction
	return load_pcs_sound(buffer, magic_number, length);
}

size_t load_music_track(byte **buffer, int32_t magic_number, int32_t length) {
	// There is no real difference between PC speaker and music in regards to extraction
	return load_pcs_sound(buffer, magic_number, length);
}

size_t extract_sound(byte **buffer, uint magic_number, sound_format format) {
	if (load_chunk_offsets() != 0) {
		return 0;
	}
	int chunk_size = 0;
	switch (format) {
		case PC_SPEAKER:
			if ((chunk_size = chunk_offsets[magic_number+1] - chunk_offsets[magic_number]) == 0) {
				fprintf(stderr, "Nonexistent sound effect: %i in format %s.\n", magic_number, format_to_string[format]);
				break;
			}
			chunk_size = load_pcs_sound(buffer, chunk_offsets[magic_number], chunk_size);
			break;
		case ADLIB_SOUND:
			magic_number += START_ADLIB_SOUND;
			if ((chunk_size = chunk_offsets[magic_number+1] - chunk_offsets[magic_number]) == 0) {
				fprintf(stderr, "Nonexistent sound effect: %i in format %s.\n", magic_number, format_to_string[format]);
				break;
			}
			chunk_size = load_adlib_sound(buffer, chunk_offsets[magic_number], chunk_size);
		default:
			fprintf(stderr, "Unknown sound effect format, aborting.");
			break;
	}
	return chunk_size;
}

size_t extract_music(byte **buffer, uint magic_number) {
	if (load_chunk_offsets() != 0) {
		return 0;
	}
	magic_number += START_MUSIC;
	int chunk_size = chunk_offsets[magic_number+1] - chunk_offsets[magic_number];
	if (chunk_size == 0) {
		fprintf(stderr, "Nonexistent music track %i.\n", magic_number);
		return 0;
	}
	DEBUG_PRINT(1, "Chunk size is %i.\n", chunk_size)

	return load_music_track(buffer, magic_number, chunk_size);
}

