#include <stdio.h>
#include "audio_extract.h"


/*-[ CONSTANTS AND MACROS ]---------------------------------------------------*/

// Error codes
#define AE_MALLOC_FAIL    1 ///< Error opening file.
#define AE_FILE_NOT_FOUND 2 ///< Error trying to allocate memory.

#define AUDIOHED_FILE "AUDIOHED.ext" ///< File containing the audio offsets.
#define AUDIOT_FILE   "AUDIOT.ext"   ///< File containing the audio chunks.
#define VSWAP_FILE    "VSWAP.EXT"    ///< File containing the digitised sound effects.

// Starting offsets for the individual sound types (see below for mappings)
#define START_PC_SOUND    (0*number_of_sounds[current_game_version]) ///< Offset to the first PC speaker chunk.
#define START_ADLIB_SOUND (1*number_of_sounds[current_game_version]) ///< Offset to the first Adlib chunk.
#define START_DIGI_SOUND  (2*number_of_sounds[current_game_version]) ///< Offset to the first digitized chunk.
#define START_MUSIC       (3*number_of_sounds[current_game_version]) ///< Offset to the first music chunk.

/** Total number of chunks to read, plus one more past the end. */
#define NUMBER_OF_CHUNKS (3*number_of_sounds[current_game_version]+number_of_music[current_game_version]+1)

/** Opens a file with given variable name and file name and keeps the file open. */ 
#define PREPARE_FILE(name, file, error)                            \
	char fname[] = file;                                           \
	change_extension(fname, extension);                            \
	FILE *name = fopen(fname, "rb");                               \
	if (name == NULL) {                                            \
		fprintf(stderr, "Error: could not open file %s.\n", fname);\
		return error;                                              \
	}                                                              \


/*-[ MAPPINGS ]---------------------------------------------------------------*/

/** Number of sound effects per type (PC speaker, Adlib, digitized). */
uint32_t number_of_sounds[GAME_VERSIONS] = {
	[WL1_I] = 87, ///< WL1 (placeholder)
	[WL3_I] = 87, ///< WL3 (placeholder)
	[WL6_I] = 87, ///< WL6
};

/** Number of digitised sound effects per game version. */
uint32_t number_of_digi_sounds[GAME_VERSIONS] = {
	[WL1_I] = 46, ///< WL1 (placeholder)
	[WL3_I] = 46, ///< WL3 (placeholder)
	[WL6_I] = 46, ///< WL6
};

/** Number of sound effects per type (PC speaker, Adlib, digitized). */
uint32_t number_of_music[GAME_VERSIONS] = {
	[WL1_I] = 27, ///< WL1 (placeholder)
	[WL3_I] = 27, ///< WL3 (placeholder)
	[WL6_I] = 27, ///< WL6
};

/** Maps a sound effect format to a string literal. */
char *format_to_string[SOUND_FORMATS] = {
	[PC_SPEAKER ] = "PC speaker",
	[ADLIB_SOUND] = "AdLib"
};


/*-[ VARIABLE DECLARATIONS ]--------------------------------------------------*/

/** Sequence of audion chunk offsets. */
uint32_t *chunk_offsets;

/** Load the offsets of the audio chunks into the offset buffer. */
int load_chunk_offsets(void);


/*-[ FUNCTION DECLARATIONS ]--------------------------------------------------*/

/** Loads a PC speaker sound effect into a buffer.
 *
 *   @param buffer  Pointer to a non-allocated byte-sequence to store the data.
 *   @param offset  Offset of the chunk in the *AUDIOT* file.
 *   @param length  Length of the chunk in the *AUDIOT* file.
 *
 *   @return  Length of the chunk, or 0 if an error occured.
 */
size_t load_pcs_sound(byte **buffer, int32_t magic_number, int32_t length);

/** Loads an AdLib sound effect into a buffer.
 *
 *  @param buffer        Pointer to a non-allocated byte-sequence to store the
 *                       data.
 *  @param magic_number  Magic number of the sound effect.
 *  @param length        Length of the chunk in the *AUDIOT* file.
 *
 *  @return  Length of the chunk, or 0 if an error occured.
 */
size_t load_adlib_sound(byte **buffer, int32_t magic_number, int32_t length);

/** Load a digitised sound into a buffer
 *
 *  @param buffer        Pointer to a non-allocated byte-sequence to store the
 *                       data.
 *  @param magic_number  Magic number of the sound effect.
 *
 *  @return  Length of the chunk, or 0 if an error occured.
 */
size_t load_digi_sound(byte **buffer, int32_t magic_number);

/** Loads a music track into a buffer.
 *
 *  @param buffer  Pointer to a non-allocated byte-sequence to store the data.
 *  @param offset  Offset of the chunk in the *AUDIOT* file.
 *  @param length  Length of the chunk in the *AUDIOT* file.
 *
 *  @return  Length of the chunk, or 0 if an error occured.
 */
size_t load_music_track(byte **buffer, int32_t magic_number, int32_t length);



/*-[ IMPLEMENTATIONS ]--------------------------------------------------------*/

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

	debug_print(1, "Loaded chunk offsets.\n");
	return 0;
}

size_t load_pcs_sound(byte **buffer, int32_t magic_number, int32_t length) {
	PREPARE_FILE(audiot, AUDIOT_FILE, 0)

	if ((*buffer = malloc(length * sizeof(byte))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory for sound effect.\n");
	}
	DEBUG_PRINT(1, "Allocated memory for sound chunk.\n")
	fseek(audiot, chunk_offsets[magic_number], SEEK_SET);
	fread(*buffer, sizeof(byte), length, audiot);
	fclose(audiot);

	debug_print(1, "Read shound chunk.\n");
	return length;
}

size_t load_adlib_sound(byte **buffer, int32_t magic_number, int32_t length) {
	// There is no real difference between PC speaker and AdLib in regards to extraction
	return load_pcs_sound(buffer, magic_number, length); // magic_number had been increased in the previous function
}

size_t load_digi_sound(byte **buffer, int32_t magic_number) {
	if (magic_number >= number_of_digi_sounds[current_game_version]) {
		fprintf(stderr, "Error: invalid digital sound number `%i`, must be within [0, %i).\n", magic_number, number_of_digi_sounds[current_game_version]);
		return 0;
	}

	word     number_of_chunks; // total number of chunks in the file
	word     sound_start;      // index of the first sound chunk in the file
	uint32_t list_offset;      // offset of the audio chunk list
	uint32_t chunk_offset;     // offset of the audio chunk
	word     chunk_length;     // length of the complete chunk
	uint32_t chunk_index;      // index of the audio chunk

	PREPARE_FILE(vswap, VSWAP_FILE, 0)
	// read the number of chunks and sound start
	fread(&number_of_chunks, sizeof(word), 1, vswap);
	fseek(vswap, 2 * sizeof(word), SEEK_SET); // skip over sprite start
	fread(&sound_start, sizeof(word), 1, vswap);

	// seek to the list, read the index of chunk and its length
	fseek(vswap, 3 * sizeof(word) + (number_of_chunks - 1) * sizeof(uint32_t), SEEK_SET);
	fread(&list_offset, sizeof(uint32_t), 1,vswap);
	fseek(vswap, list_offset * sizeof(byte) + magic_number * 2 * sizeof(word), SEEK_SET);
	fread(&chunk_index, sizeof(word), 1, vswap);
	fread(&chunk_length, sizeof(word), 1, vswap);

	// seek to the location storing the chunk offset
	fseek(vswap, 3*sizeof(word) + (sound_start + chunk_index) * sizeof(uint32_t), SEEK_SET);
	fread(&chunk_offset, sizeof(uint32_t), 1, vswap);
	
	// seek to the chunk, allocate buffer and read it
	fseek(vswap, chunk_offset * sizeof(byte), SEEK_SET);
	if ((*buffer = malloc(chunk_length * sizeof(byte))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory for sound effect.\n");
		fclose(vswap);
		return 0;
	}
	fread(*buffer, sizeof(byte), chunk_length, vswap);

	fclose(vswap);
	return chunk_length;
}

size_t load_music_track(byte **buffer, int32_t magic_number, int32_t length) {
	// There is no real difference between PC speaker and music in regards to extraction
	return load_pcs_sound(buffer, magic_number, length); // magic_number had been increased in the previous function
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
			chunk_size = load_pcs_sound(buffer, magic_number, chunk_size);
			break;
		case ADLIB_SOUND:
			magic_number += START_ADLIB_SOUND;
			if ((chunk_size = chunk_offsets[magic_number+1] - chunk_offsets[magic_number]) == 0) {
				fprintf(stderr, "Nonexistent sound effect: %i in format %s.\n", magic_number, format_to_string[format]);
				break;
			}
			chunk_size = load_adlib_sound(buffer, magic_number, chunk_size);
			break;
		case DIGI_SOUND:
			chunk_size = load_digi_sound(buffer, magic_number);
			break;
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
	debug_print(1, "Chunk size is %i.\n", chunk_size);

	return load_music_track(buffer, magic_number, chunk_size);
}

