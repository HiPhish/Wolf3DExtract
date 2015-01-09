/** @file snd2wlf.c
 *
 *  Converter program that converts Wolfenstein 3D AdLib sound effects to WLF
 *  format files.
 *  
 *  The AdLib sound effects are stored in a format that's similar to, but not
 *  exactly the Wolfenstein 3D music format. The audio data was meant to be
 *  sent to the sound hardware in a very specific way, so only the variable
 *  data was stored. This converter adds the omitted information back so the
 *  sound can be played like a music track.
 *
 *  The format of WLF music files is a variation of the IMF (id music format)
 *  specification. The only difference is that the lengh of a tick in an IMF
 *  file is 1/560 seconds, while it is 1/700 seconds in a WLF file.
 *
 *  Here is the structure of the AdLib sound effect files:
 *
 *  | Type         | Name       | Description                    |
 *  |--------------|------------|--------------------------------|
 *  | Uint32le     | length     | Length of the audio data       |
 *  | Uint16le     | priority   | Priority of the sound effect   |
 *  | Byte[16]     | instrument | Instrument settings            |
 *  | Byte         | octave     | Octave number                  |
 *  | Byte[length] | data       | Actual sound data              |
 *  | Uint8        | terminator | Unused                         |
 *  | Char[]       | name       | Null-terminated name, optional |
 *
 *  The length and priority are *header* data and irrelevant to the actual
 *  effect, they only serve for the interpreting program to know how to play
 *  back the sound effect. The terminator and the name likewise are a *footer*
 *  and only relevant to *Muse*, the program they were created with.
 *
 *  The original source of the program was written by Sune Mindall in Java and
 *  has been ported to C by HiPhish.
 */

#include <stdint.h>
#include <stdio.h>


/*-[ CONSTANTS ]--------------------------------------------------------------*/

#define MIN_DELAY   0 ///< The shortest delay that can be used.
#define TICK_DELAY  5 ///< One AdLib-sound-tick(@140hz) is 5 WLF ticks(@700Hz)

/** List of AdLib registers.
 *
 *  The reason I don't just make the numeric value of the items the hardware
 *  addresses right away is so I could be able to iterate over the sequence of
 *  registers when reading and writing in one go.
 */
enum adlib_registers {
	M_CHAR   ,///< Modulator characteristics.
	C_CHAR   ,///< Carrier characteristics.
	M_SCALE  ,///< Modulator scale.
	C_SCALE  ,///< Carrier scale.
	M_ATTACK ,///< Modulator attack/decay rate.
	C_ATTACK ,///< Carrier attack/decay rate.
	M_SUST   ,///< Modulator sustain.
	C_SUST   ,///< Carrier sustain.
	M_WAVE   ,///< Modulator waveform.
	C_WAVE   ,///< Carrier waveform.
	N_CONN   ,///< Feedback/connection (usually ignored and set to 0).

	NUMBER_OF_REGISTERS
};

/*-[ MAPPINGS ]---------------------------------------------------------------*/

/** Map an AdLib register to its hardware address */
uint8_t register_adress[NUMBER_OF_REGISTERS] = {
	[M_CHAR  ] = 0x20,
	[C_CHAR  ] = 0x23,
	[M_SCALE ] = 0x40,
	[C_SCALE ] = 0x43,
	[M_ATTACK] = 0x60,
	[C_ATTACK] = 0x63,
	[M_SUST  ] = 0x80,
	[C_SUST  ] = 0x83,
	[M_WAVE  ] = 0xE0,
	[C_WAVE  ] = 0xE3,
	[N_CONN  ] = 0xC0,
};


/*-[ FUNCTION DECLARATIONS ]--------------------------------------------------*/

/** Convert an AdLib sound effect file from the standard input to a WLF music
 *  file to the standard output.
 */
int convert_sound(void);

/** Write an IMF data element to the standard output.
 *
 *  @param reg      The OPL register to write.
 *  @param data     Data to write to the register.
 *  @param delay    Delay before next element, specified in ticks.
 *  @param counter  Pointer to a counter integer to keep track up how many
 *                  bytes we have written.
 * 
 *  An IMF data element consists of four bytes:
 *
 *  - byte 1:    OPL register
 *  - byte 2:    Data to write to the register
 *  - byte 3+4:  Delay before next element, specified in ticks
 *
 *  A tick is either 1/560 sec for regular IMF or 1/700 sec for WLF.
 */
void write_imf_element(uint8_t opl, uint8_t data, uint16_t delay, uint16_t *counter);

/** Handles the arguments supplied to the program.
 *  
 *  @param argc  Number of arguments passed.
 *  @param argv  Array of argument literals passed.
 */
void handle_arguments(int argc, char *argv[]);

/** Prints usage instructions to the standard output. */
void print_usage(void);

/** Reads an unsigned 8-bit integer from file in an endian-independent way.
 *
 *  @return  The integer that was read.
 * 
 *  Reading advances the file position pointer.
 */
uint32_t read_uint8(void);

/** Reads an unsigned 16-bit integer from file in an endian-independent way.
 *
 *  @return  The integer that was read.
 *
 *  Reading advances the file position pointer.
 */
uint32_t read_uint16(void);

/** Reads an unsigned 32-bit integer from file in an endian-independent way.
 *
 *  @return  The integer that was read.
 *
 *  Reading advances the file position pointer.
 */
uint32_t read_uint32(void);

/** Write an unsigned 16-bit integer to the standard output.
 *
 *  @param i  The integer to write.
 */
void write_uint16(uint16_t i);


/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
	return convert_sound();
}
/*----------------------------------------------------------------------------*/


/*-[ FUNCTION IMPLEMENTATIONS ]-----------------------------------------------*/

int convert_sound() {
	//Read length of the sound data from the header
	uint32_t length = read_uint32();
	
	// Write placeholder for the length of the output
	write_uint16(0x0000);

	// The real length of the WLF music data
	uint16_t wlf_length = 0;
	
	//Read priority
	uint16_t priority = read_uint16();
	
	// Read and write instrument data
	// nConn must never be set or the sound will play wrong
	for (int reg = 0; reg < NUMBER_OF_REGISTERS - 1; ++reg) {
		uint8_t intstrument = read_uint8();
		write_imf_element(register_adress[reg], intstrument, MIN_DELAY, &wlf_length);
	}
	
	// The last six bytes of the sequence are either padding or unused, skip ahead
	fseek(stdin, 6, SEEK_CUR);

	// Read octave and compute block value from it
	uint8_t block = (read_uint8() & 7) << 2;

	// Read and write the pitch data
	int note_on = 0; //boolean variable
	for (int i = 0; i < length;) {
		uint8_t note_value = read_uint8();
		++i;
		uint16_t repeated = 1;
		//look ahead to see if the pitch is repeated. If so, the repetitions is counted and we skip ahead. 
		while (i < length && read_uint8() == note_value) {
			++repeated;
			++i;
		}
		// step back one byte (remember reading bytes moves forward through the file)
		fseek(stdin, -1, SEEK_CUR);
		if (note_value == 0x00) { // note not to be played
			write_imf_element(0xB0, block, TICK_DELAY * repeated, &wlf_length);
			note_on = 0;
		}
		else if (!note_on) { // note to be played and status noteOFF
			write_imf_element(0xA0, note_value  , MIN_DELAY            , &wlf_length);
			write_imf_element(0xB0, block | 0x20, TICK_DELAY * repeated, &wlf_length);
			note_on = 1;
		} else { // note to be played and status noteON
			write_imf_element(0xA0, note_value, TICK_DELAY * repeated, &wlf_length);
			note_on = 1;
		}
	}
	// Add final note off
	write_imf_element(0xB0, block, MIN_DELAY, &wlf_length);

	// skip terminator (fseek has stepped one byte back, so we have to skip two bytes forward)
	fseek(stdin, 2, SEEK_CUR);

	// The audio data has been written, the rest is tag data that will be ignored
	// by audio players. As such the data is not strictly standardised
	
	// The purpose of this is unknown
	fprintf(stdout, "%c%c", '\0', '\0');

	//Read name(from footer) and write it down (the name is always 16 characters)
	for (int i = 0; i < 15; ++i) {
		char c = read_uint8();
		fprintf(stdout, "%c", c);
		// reading the null character, that's the end of the string so break out
		if (c  == '\0') {
			// pad the rest of the string with '\0' to 16 characters
			for (int j = 0; j < 15 - i - 1; ++j) {
				fprintf(stdout, "%c", '\0');
			}
			break;
		}
	}
	fprintf(stdout, "%c", '\0'); // terminate the string

	// write comment (64 chars) and cProg (6 chars)
	for (int i = 0; i < 70; ++i) {
		fprintf(stdout, "%c", '\0');
	}

	// finally, write the WLF audio data length
	fseek(stdout, 0, SEEK_SET); // seek to the beginning of the file
	write_uint16(wlf_length);

	return 0;
}


void write_imf_element(uint8_t opl, uint8_t data, uint16_t delay, uint16_t *counter) {
	fwrite(&opl , sizeof(uint8_t), 1, stdout);
	fwrite(&data, sizeof(uint8_t), 1, stdout);
	write_uint16(delay);
	if (counter != NULL) {*counter += 4;}
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

void write_uint16(uint16_t i) {
	uint8_t le = i & 0x00FF;  // little end
	uint8_t be = i >> 8;      // big end
	fwrite(&le, sizeof(uint8_t), 1, stdout);
	fwrite(&be, sizeof(uint8_t), 1, stdout);
}

