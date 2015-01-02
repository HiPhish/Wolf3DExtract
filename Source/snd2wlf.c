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
 */

#include <stdint.h>
#include <stdio.h>

#define MIN_DELAY  0
#define TICK_DELAY 5 ///< One AdLib-sound-tick(@140hz) is 5 IMF ticks(@700Hz)


//offsets in AdLib-soundchunk
#define DATASZ_OFFSET    0 ///< uint32_t ie length 4
#define PRIORITY_OFFSET  4 ///< uint16_t ie length 2
#define INSTR_OFFSET     6 ///< uint8_t[16] (uint 8 x 16) length 16
#define OCT_OFFSET      22 ///< uint8_t length = 1
#define DATA_OFFSET     23 ///< uint8_t[*] length specified in dataSz.

//AdLib registers (all bytes)
#define M_CHAR   ((uint8_t) 0x20) ///< Modulator characteristics.
#define C_CHAR   ((uint8_t) 0x23) ///< Carrier characteristics.
#define M_SCALE  ((uint8_t) 0x40) ///< Modulator scale.
#define C_SCALE  ((uint8_t) 0x43) ///< Carrier scale.
#define M_ATTACK ((uint8_t) 0x60) ///< Modulator attack/decay rate.
#define C_ATTACK ((uint8_t) 0x63) ///< Carrier attack/decay rate.
#define M_SUST   ((uint8_t) 0x80) ///< Modulator sustain.
#define C_SUST   ((uint8_t) 0x83) ///< Carrier sustain.
#define M_WAVE   ((uint8_t) 0xE0) ///< Modulator waveform.
#define C_WAVE   ((uint8_t) 0xE3) ///< Carrier waveform.
#define N_CONN   ((uint8_t) 0xC0) ///< Feedback/connection (usually ignored and set to 0).

int convert_sound(void);

/** Write an IMF data element to the standard output.
 *
 *  @param reg
 *  @param data
 *  @param delay
 * 
 *  An IMF data element consists of for bytes:
 *  - byte 1:    OPL register
 *  - byte 2:    Data to write to the register
 *  - byte 3+4:  Delay before next element [UIntLE16]  -- this is specified in ticks
 *  A tick is either 1/560 sec for regular IMF or 1/700 sec for WLF.
 */
void write_imf_element(uint8_t reg, uint8_t data, int delay);

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

int main(int argc, char *argv[]) {
	return 0;
}

int convert_sound() {
	uint32_t data_size = read_uint32();
	uint16_t priority  = read_uint16();
	uint8_t  intstrument[16];
	for (int i = 0; i < 16; ++i) {
		intstrument[i] = read_uint8();
	}
	uint8_t  octave = read_uint8();
	uint8_t *data;


	//Write blank dataSize in imf-header (we don't know this yet.) This is a UInt16LE
	uint16_t imf_length = 0x0000;

	return 0;
}

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


