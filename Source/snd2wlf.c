/** @file snd2wlf.c
 *
 *  Converter program that converts Wolfenstein 3D AdLib sound effects to WLF
 *  format files.
 *  
 *  The AdLib sound effects are stored in a format that's similar to, but not
 *  exactly the Wolfenstein 3D music format.
 *
 *  The original source of the program was written by Sune Mindall in Java and
 *  has been converted to C by HiPhish.
 *
 *  **NOTE:** This program is still work in progress.
 *
 *  The format of WLF music files is a variation of the IMF (id music format)
 *  specification. The only difference is that the lengh of a tick in an IMF
 *  file is 1/560 seconds, while it is 1/700 seconds in a WLF file.
 *
 *  Here is the structure of the AdLib sound effect files:
 *
 *  | Type         | Name       | Description                    |
 *  |--------------|------------|--------------------------------|
 *  | Uint32le     | length     | Length of the sound data       |
 *  | Uint16le     | priority   | Priority of the sound effect   |
 *  | Byte[16]     | instrument | Instrument settings            |
 *  | Byte         | octave     | Octave number                  |
 *  | Byte[length] | data       | Actual sound data              |
 *  | Uint8        | terminator | Unused?                        |
 *  | Char[]       | name       | null-terminated name, optional |
 *
 *  The length and priority are *header* data and irrelevant to the actual
 *  effect, they only serve for the interpreting program to know how to play
 *  back the sound effect. The terminator and the name likewise are a *footer*
 *  and only relevant to *MUSE*, the program they were created with.
 */

#include <stdint.h>
#include <stdio.h>


/*-[ CONSTANTS ]--------------------------------------------------------------*/

#define MIN_DELAY   0 ///< asdf
#define TICK_DELAY  5 ///< One AdLib-sound-tick(@140hz) is 5 WLF ticks(@700Hz)

//offsets in AdLib-soundchunk
#define DATASZ_OFFSET     0 ///< uint32_t ie length 4
#define PRIORITY_OFFSET   4 ///< uint16_t ie length 2
#define INSTR_OFFSET      6 ///< uint8_t[16] (uint 8 x 16) length 16
#define OCT_OFFSET       22 ///< uint8_t length = 1
#define DATA_OFFSET      23 ///< uint8_t[*] length specified in dataSz.

//AdLib registers (all bytes)
#define M_CHAR    ((uint8_t)0x20) ///< Modulator characteristics.
#define C_CHAR    ((uint8_t)0x23) ///< Carrier characteristics.
#define M_SCALE   ((uint8_t)0x40) ///< Modulator scale.
#define C_SCALE   ((uint8_t)0x43) ///< Carrier scale.
#define M_ATTACK  ((uint8_t)0x60) ///< Modulator attack/decay rate.
#define C_ATTACK  ((uint8_t)0x63) ///< Carrier attack/decay rate.
#define M_SUST    ((uint8_t)0x80) ///< Modulator sustain.
#define C_SUST    ((uint8_t)0x83) ///< Carrier sustain.
#define M_WAVE    ((uint8_t)0xE0) ///< Modulator waveform.
#define C_WAVE    ((uint8_t)0xE3) ///< Carrier waveform.
#define N_CONN    ((uint8_t)0xC0) ///< Feedback/connection (usually ignored and set to 0).


/*-[ FUNCTION DECLARATIONS ]--------------------------------------------------*/

int convert_sound(void);

/** Write an IMF data element to the standard output.
 *
 *  @param reg    The OPL register to write.
 *  @param data   Data to write to the register.
 *  @param delay  Delay before next element, specified in ticks.
 * 
 *  An IMF data element consists of four bytes:
 *
 *  - byte 1:    OPL register
 *  - byte 2:    Data to write to the register
 *  - byte 3+4:  Delay before next element, specified in ticks
 *
 *  A tick is either 1/560 sec for regular IMF or 1/700 sec for WLF.
 */
void write_imf_element(uint8_t reg, uint8_t data, uint16_t delay);

/** Reads an unsigned 8-bit integer from file in an endian-independent way.
 *
 *  @return  The integer that was read.
 * 
 *  Reading an integer advances the file position by the data type size of the integer.
 */
uint32_t read_uint8(void);

/** Reads an unsigned 16-bit integer from file in an endian-independent way.
 *
 *  @return  The integer that was read.
 *
 *  Reading an integer advances the file position by the data type size of the integer.
 */
uint32_t read_uint16(void);

/** Reads an unsigned 32-bit integer from file in an endian-independent way.
 *
 *  @return  The integer that was read.
 *
 *  Reading an integer advances the file position by the data type size of the integer.
 */
uint32_t read_uint32(void);


/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
	return 0;
}
/*----------------------------------------------------------------------------*/


/*-[ FUNCTION IMPLEMENTATIONS ]-----------------------------------------------*/

int convert_sound() {
	//Read input data size (from header)
	uint32_t data_size = read_uint32();
	
	//Read priority
	uint16_t priority  = read_uint16();
	
	// Read and write instrument data
	uint8_t  intstrument[16];
	for (int i = 0; i < 16; ++i) {
		intstrument[i] = read_uint8();
	}

	// Read octave and calculate block value
	uint8_t block = read_uint8() << 2;

	// Read and write the pitch data
	int note_on = 0, i = DATA_OFFSET;
	while (i < DATA_OFFSET + data_size) {
		uint8_t note_value = read_uint8();
		++i;
		int repeated = 1;
		//look ahead to see if the pitch is repeated. If so, the repetitions is counted and we skip ahead. 
		while ((i+1) < DATA_OFFSET + data_size && read_uint8() == note_value) {
			++repeated;
			++i;
		}
		// step back one byte (remember reading bytes moves forward through the file)
		fseek(stdin, -1, SEEK_CUR);
		if (note_value == 0x00) { // note not to be played
			write_imf_element(0xB0, block, TICK_DELAY * repeated);
			note_on = 0;
		}
		else if (!note_on) { // note to be played and status noteOFF
			write_imf_element(0xA0, note_value  , MIN_DELAY            );
			write_imf_element(0xB0, block | 0x20, TICK_DELAY * repeated);
			note_on = 1;
		} else { // note to be played and status noteON
			write_imf_element(0xA0, note_value, TICK_DELAY * repeated);
			note_on = 1;
		}
	}
	// Add final note off
	write_imf_element(0x0B, block, MIN_DELAY);

	//Read name(from footer) and priority

	//Write blank dataSize in imf-header (we don't know this yet.) This is a UInt16LE
	uint16_t imf_length = 0x0000;

	return 0;
}


// In order to be able to read multi-byte value in an endian-independen manner
// we first read the individual bytes in their original order, then we OR them
// bitwise like a little-endian number.

uint32_t read_uint8() {
	uint8_t byte[1] = {0x00};
	fread(byte, sizeof(uint8_t), 1, stdin);
	return byte[0];
}

uint32_t read_uint16() {
	uint8_t byte[2] = {0x00, 0x00};
	fread(byte, sizeof(uint8_t), 2, stdin);
	uint32_t result = ((uint16_t)byte[0])<<0 | ((uint16_t)byte[1])<<8;
	return result;
}

uint32_t read_uint32() {
	uint8_t byte[4] = {0x00, 0x00, 0x00, 0x00};
	fread(byte, sizeof(uint8_t), 4, stdin);
	uint32_t result = ((uint32_t)byte[0])<<0 | ((uint32_t)byte[1])<<8 | ((uint32_t)byte[2])<<16| ((uint32_t)byte[3])<<24;
	return result;
}


