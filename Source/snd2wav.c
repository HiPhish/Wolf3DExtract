/**
 * @file pcs2wav.c
 *
 * Converter program to convert Wolfenstein 3D pc sound effects to Wave files.
 *
 * A simple RIFF wave file consists of a file header for the entire file and two
 * chunks: format and data. All numbers are little-endian. This is what the file
 * header looks like:
 *
 * | Type    | Name        | Description                       |
 * |---------|-------------|-----------------------------------|
 * | char[4] | declaration | The string "RIFF" in ASCII        |
 * | uint_32 | file length | Length of the entire file minus 8 |
 * | char[4] | declaration | The string "WAVE" in ASCII        |
 *
 * The header is immediately followed by the format chunk, which has always the
 * same size in our case.
 *
 * | Type    | Name         | Decription                                                  |
 * |---------|--------------|-------------------------------------------------------------|
 * | char[4] | declaration  | The string "fmt " in ASCII (note the space character!)      |
 * | uint_32 | size         | Size of the rest of the format chunk in bytes, always 16    |
 * | uint_16 | audio format | Always 1 for PCM audio                                      |
 * | uint_16 | channels     | How many channels, 1 for mono in our case                   |
 * | uint_32 | sample rate  | Number of samples per second                                |
 * | uint_32 | byte rate    | Number of bytes per second (sampe_rate*channels*bit_rate/8) |
 * | uint_16 | block align  | channels * bit_rate / 8                                     |
 * | uint_32 | bit rate     | Number of bits per second (here always 8)                   |
 *
 * It is then followed by the last chunk: the data chunk
 *
 * | Type     | Name        | Description                               |
 * |----------|-------------|-------------------------------------------|
 * | char [4] | declaration | The string "data" in ASCII                |
 * | uint_32  | size        | Size of the following data chunk in bytes |
 * | uint_8[] | data        | The actual audio data                     |
 *
 * We can see that the header is always 12 bytes long, the format chunk 24 bytes
 * and the data chunk 8 bytes plus the size of audio data. Some specifiations
 * don't count the first two items of the format- and data chunk, so they will
 * list the length of those chunks as 16 and the length of the audio data.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef unsigned int uint;


/*-[ CONSTANTS ]--------------------------------------------------------------*/

// Sound effect format
#define DIGITISED         0 ///< Digitised sound effect.
#define PC_SPEAKER        1 ///< PC Speaker sound effect.
#define NUMBER_OF_FORMATS 2 ///< Number of possible sound effect formats.

#define DIGITIZED DIGITISED ///< Alternate spelling of `DIGITISED`.

#define BASE_TIMER 1193181 ///< Inverse proportional factor for playback: `frequency = 1193181 / value`.
#define PCS_RATE       140 ///< Playback rate of original hardware in bytes/second.
#define PCS_VOLUME      20 ///< Audio volume of the simulated speaker.

/*-[ FUNCTION DECLARATIONS ]--------------------------------------------------*/

/** Read the digitised sound data into a buffer.
 *
 *  @param snd_length  Pointer to store the length of the audio data buffer.
 *  @param snd_buffer  Pointer to the pointer of the audio data sequence.
 *
 *  @return  0 if everything went right, non-0 otherwise.
 *
 *  The length and the buffer pointer are entered blank and will be assigned
 *  values.
 */
int read_digi_buffer(size_t *snd_length, uint8_t **snd_buffer);

/** Read the PC speaker sound data into a buffer.
 *
 *  @param snd_length  Pointer to store the length of the audio data buffer.
 *  @param snd_buffer  Pointer to the pointer of the audio data sequence.
 *
 *  @return  0 if everything went right, non-0 otherwise.
 *
 *  The length and the buffer pointer are entered blank and will be assigned
 *  values.
 */
int read_pcs_buffer(size_t *snd_length, uint8_t **snd_buffer);

/** Convert digitised sound effect audio data to wave file format.
 *
 *  @param source       Pointer to input sequence.
 *  @param destination  Pointer to output sequence.
 *  @param length       Length of the input sequence.
 *  @param sample_rate  How many samples to generate for each second.
 *
 *  @return  0 if everything went right, non-0 otherwise.
 *
 *  The output is raw PCM audio data, not a complete wave file. To create a wave
 *  file additional data has to be added.
 */
int digi_to_wave(uint8_t **source, uint8_t **destination, size_t snd_length, uint32_t sample_rate);

/** Converts a sequence of PC speaker audio data to wave file format.
 *
 *  @param source       Pointer to input sequence.
 *  @param destination  Pointer to output sequence.
 *  @param length       Length of the input sequence.
 *  @param sample_rate  How many samples to generate for each second.
 *
 *  @return  0 if everything went right, non-0 otherwise.
 *
 *  The output is raw PCM audio data, not a complete wave file. To create a wave
 *  file additional data has to be added.
 */
int pcs_to_wave(uint8_t **source, uint8_t **destination, size_t snd_length, uint32_t sample_rate);

/** Processes arguments.
 *
 *  @param argc  Number of arguments to process.
 *  @param argv  Array of argument strings.
 *
 *  The last argument wins, unknown arguments do nothing.
 */
void process_arguments(int argc, char *argv[]);

/** Print usage instructions to standard error. */
void print_usage(void);


/*-[ GLOBAL VARIABLE DEFINTIONS ]---------------------------------------------*/

/** Format of the audio data (digitised or PC speaker). */
int audio_format = DIGITISED;

/** Map audio format to function to read the audio data from file. */
int (*read_snd_buffer[NUMBER_OF_FORMATS])(size_t *snd_length, uint8_t **snd_buffer) = {
	[DIGITISED ] = read_digi_buffer, ///< Digitised audio.
	[PC_SPEAKER] = read_pcs_buffer , ///< PC speaker.
};

/** Map audio format to function to convert audio data to wave data. */
int (*snd_to_wave[NUMBER_OF_FORMATS])(uint8_t **source, uint8_t **destination, size_t snd_length, uint32_t sample_rate) = {
	[DIGITISED ] = digi_to_wave, ///< Digitised sound.
	[PC_SPEAKER] = pcs_to_wave , ///< PC speaker.
};

/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
	process_arguments(argc, argv);
	uint8_t *snd_buffer; // sound data sequence (bytes)
	uint8_t *wav_buffer; // audio data sequence (bytes)
	size_t   snd_length; // length of the audio data sequence in bytes
	size_t   wav_length; // length of the wave data sequence in bytes
	
	if (read_snd_buffer[audio_format](&snd_length, &snd_buffer) != 0 || snd_length == 0) {
		fprintf(stderr, "Error: could not load raw audio data of length %i.\n", (int)snd_length);
		return 1;
	}
	
	if ((wav_length = snd_to_wave[audio_format](&snd_buffer, &wav_buffer, snd_length, 40000)) == 0) {
		free(snd_buffer);
		free(wav_buffer);
		return 1;
	}
	free(snd_buffer); // the sound buffer is no longer needed

	// Print wave file header
	uint32_t file_size = 36 + wav_length; // Length of the entire file minus 8

	uint8_t format_length[4] = {0x10, 0x00, 0x00, 0x00}; //     16
	uint8_t format_type  [2] = {0x01, 0x00            }; //      1
	uint8_t channels     [2] = {0x01, 0x00            }; //      1
	uint8_t sample_rate  [4] = {0x40, 0x9C, 0x00, 0x00}; // 40,000
	uint8_t byte_rate    [4] = {0x40, 0x9C, 0x00, 0x00}; // 40,000
	uint8_t block_align  [2] = {0x01, 0x00            }; //      1
	uint8_t bit_rate     [2] = {0x08, 0x00            }; //      8

	// digitised sound has a different sample rate, so some values need to be changed
	if (audio_format == DIGITISED) { // 7000
		sample_rate[0] = 0x58;
		sample_rate[1] = 0x1B;
		
		byte_rate[0] = 0x58;
		byte_rate[1] = 0x1B;
	}


	fwrite("RIFF", sizeof(char), 4, stdout);
	fwrite(&file_size, sizeof(uint32_t), 1, stdout); // Needs to be total file size - 8
	fwrite("WAVE", sizeof(char), 4, stdout);

	// Print format chunk
	fwrite("fmt ", sizeof(char), 4, stdout);
	fwrite(format_length, sizeof(uint8_t), 4, stdout);
	fwrite(format_type  , sizeof(uint8_t), 2, stdout);
	fwrite(channels     , sizeof(uint8_t), 2, stdout);
	fwrite(sample_rate  , sizeof(uint8_t), 4, stdout);
	fwrite(byte_rate    , sizeof(uint8_t), 4, stdout);
	fwrite(block_align  , sizeof(uint8_t), 2, stdout);
	fwrite(bit_rate     , sizeof(uint8_t), 2, stdout);

	// Print data chunk
	fwrite("data", sizeof(char), 4, stdout);
	fwrite(&wav_length, sizeof(uint32_t), 1, stdout);
	fwrite(wav_buffer, sizeof(uint8_t), wav_length, stdout);

	free(wav_buffer);
	return 0;
}
/*----------------------------------------------------------------------------*/

int read_digi_buffer(size_t *snd_length, uint8_t **snd_buffer) {
	fread(snd_length, sizeof(uint16_t), 1, stdin);
	if ((*snd_buffer = malloc(*snd_length * sizeof(uint8_t))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory for digitised sound effect.\n");
		return 1;
	}
	fread(*snd_buffer, sizeof(uint8_t), *snd_length, stdin);
	return 0;
}

int read_pcs_buffer(size_t *snd_length, uint8_t **snd_buffer) {
	fread(snd_length, sizeof(uint32_t), 1, stdin);
	if ((*snd_buffer = malloc(*snd_length * sizeof(uint8_t))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory for PC sound effect.\n");
		return 1;
	}
	fseek(stdin, sizeof(uint16_t), SEEK_CUR); // skip over priority word
	fread(*snd_buffer, sizeof(uint8_t), *snd_length, stdin);
	return 0;
}

int digi_to_wave(uint8_t **source, uint8_t **destination, size_t snd_length, uint32_t sample_rate) {
	if ((*destination = malloc(snd_length * sizeof(uint8_t))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory to hold wave audio data.\n");
		return 0;
	}
	memcpy(*destination, *source, snd_length);
	return snd_length;
}

// this code is from the [Modding Wiki](http://www.shikadi.net/moddingwiki/AudioT_Format#Code:_Converting_to_Wave_format)
int pcs_to_wave(uint8_t **source, uint8_t **destination, size_t snd_length, uint32_t sample_rate) {
	uint32_t samples_per_byte = sample_rate / PCS_RATE;
	size_t   wav_length       = snd_length * samples_per_byte * sizeof(uint8_t);

	if ((*destination = malloc(wav_length)) == NULL) {
		fprintf(stderr, "Error: could not allocate memory for wave file.\n");
		return 0;
	}

	uint8_t *read  = *source;      // read head for input sequence
	uint8_t *write = *destination; // write head for output sequence

	int sign = -1;

	uint tone            ,
		 phase_length    ,
		 phase_tick   = 0;

	while (snd_length--) {
		tone = *(read++) * 60; // the value 60 is hard-coded
		phase_length = (sample_rate * tone) / (2 * BASE_TIMER);
		for (int i = 0; i < samples_per_byte; ++i) {
			if (tone) {
				*(write++) = 128 + sign * PCS_VOLUME;
				if (phase_tick++ >= phase_length) {
					sign *= -1;
					phase_tick = 0;
				}
			} else {
				phase_tick = 0;
				*(write++) = 128;
			}
		}
	}
	
	return wav_length;
}

void process_arguments(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		if (strncmp(argv[i], "-p", 2) == 0) {
			audio_format = PC_SPEAKER;
		} else if (strncmp(argv[i], "-d", 2) == 0) {
			audio_format = DIGITISED;
		} else {
			fprintf(stderr, "Error: unkown argument \"%s\".\n", argv[i]);
			print_usage();
		}
	}
}

void print_usage(void) {
	fprintf(stderr, "Usage: input is the standard input, output is the standard output. Use the\n"
			        "following arguments:\n"
					"  -digi  Digitised audio mode (default)\n"
			        "  -pc    PC speaker mode\n"
		   );
}

