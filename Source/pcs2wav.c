/*
 * @file pcs2wav.c
 *
 * Converter program to convert Wolfenstein 3D pc sound effects to Wave files.
 *
 * A simple RIFF wave file consists of a file header for the entire file and two chunks: format and data. All numbers are little-endian.
 * This is what the file header looks like:
 * | Type    | Name        | Description                       |
 * |---------|-------------|-----------------------------------|
 * | char[4] | declaration | The string "RIFF" in ASCII        |
 * | uint_32 | file length | Length of the entire file minus 8 |
 * | char[4] | declaration | The string "WAVE" in ASCII        |
 *
 * The header is immediately followed by the format chunk, which has always the same size in our case.
 * | Type    | Name         | Decription                                                  |
 * |---------|--------------|-------------------------------------------------------------|
 * | char[4] | declaration  | The string "fmt " in ASCII (note the space character!)      |
 * | uint_32 | size         | Size of the rest of the format chunk in bytes, always 16    |
 * | uint_16 | audio format | Always 1 for PCM audio                                      |
 * | uint_16 | channels     | How many channels, 1 for mono                               |
 * | uint_32 | sample rate  | Number of samples per second                                |
 * | uint_32 | byte rate    | Number of bytes per second (sampe_rate*channels*bit_rate/8) |
 * | uint_16 | block align  | channels * bit_rate / 8                                     |
 * | uint_32 | bit rate     | Number of bits per second (here always 8)                   |
 *
 * It is then followed by the last chunk: the data chunk
 * | Type     | Name        | Description                               |
 * |----------|-------------|-------------------------------------------|
 * | char [4] | declaration | The string "data" in ASCII                |
 * | uint_32  | size        | Size of the following data chunk in bytes |
 * | uint_8[] | data        | The actual audio data                     |
 *
 * We can see that the header is always 12 bytes long, the format chunk 24 bytes and the data chunk 8 bytes plus the size of audio data. Some
 * specifiations don't count the first two items of the format- and data chunk, so they will list the length of those chunks as 16 and the
 * length of the audio data.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef int uint;

#define PCS_BASE_TIMER 1193181 ///< Inverse proportional factor for playback: `frequency = 1193181 / value`.
#define PCS_RATE           140 ///< Playback rate of original hardware in bytes/second.
#define PCS_VOLUME          20

/**
 * Converts a sequence of PC speaker audio data to wave file format.
 *
 * @param source      Pointer to input sequence.
 * @param destination Pointer to output sequence.
 * @param length      Length of the input sequence.
 * @param frequency
 *
 * The output is raw PCM audio data, not a complete wave file. To create a wave
 * file additional data has to be added.
 */
int convert_audio(uint8_t **source, uint8_t **destination, size_t pcs_length, uint32_t sample_rate);

int main(int argc, char *argv[]) {
	size_t   pcs_length;
	size_t   wav_length;
	uint8_t *pcs_buffer;
	uint8_t *wav_buffer;
	
	fread(&pcs_length, sizeof(uint32_t), 1, stdin);
	if ((pcs_buffer = malloc(pcs_length * sizeof(uint8_t))) == NULL) {
		fprintf(stderr, "Error: could not allocate memory for PC sound effect");
		return 1;
	}
	fseek(stdin, sizeof(uint16_t), SEEK_CUR);
	fread(pcs_buffer, sizeof(uint8_t), pcs_length, stdin);
	
	if ((wav_length = convert_audio(&pcs_buffer, &wav_buffer, pcs_length, 40000)) == 0) {
			free(pcs_buffer);
			free(wav_buffer);
			return 1;
	}

	free(pcs_buffer);

	// Print wave file header
	uint32_t file_size = 36 + wav_length; // Length of the entire file minus 8

	uint8_t format_length[4] = {0x10, 0x00, 0x00, 0x00}; //     16
	uint8_t format_type  [2] = {0x01, 0x00            }; //      1
	uint8_t channels     [2] = {0x01, 0x00            }; //      1
	uint8_t sample_rate  [4] = {0x40, 0x9C, 0x00, 0x00}; // 40,000
	uint8_t byte_rate    [4] = {0x40, 0x9C, 0x00, 0x00}; // 40,000
	uint8_t block_align  [2] = {0x01, 0x00            }; //      1
	uint8_t bit_rate     [2] = {0x08, 0x00            }; //      8


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

// this code is from the [Modding Wiki](http://www.shikadi.net/moddingwiki/AudioT_Format#Code:_Converting_to_Wave_format)
int convert_audio(uint8_t **source, uint8_t **destination, size_t pcs_length, uint32_t sample_rate) {
	size_t wav_length = pcs_length * (sample_rate / PCS_RATE) * sizeof(uint8_t);
	if ((*destination = malloc(wav_length)) == NULL) {
		fprintf(stderr, "Error: could not allocate memory for wave file.\n");
		return 0;
	}
	uint8_t *read  = *source;      // read head for input sequence
	uint8_t *write = *destination; // write head for output sequence

	int sign = -1;

	uint tone,
		 phase_length,
		 phase_tick       = 0,
		 samples_per_byte = sample_rate / PCS_RATE;

	while (pcs_length--) {
		tone = *(read++) * 60; // the vlaue 60 is hard-coded
		phase_length = (sample_rate * tone) / (2 * PCS_BASE_TIMER);
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

