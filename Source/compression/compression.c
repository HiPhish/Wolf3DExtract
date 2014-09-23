#include <stdio.h>
#include <stdlib.h>
#include "compression.h"

#define NEAR 0xA7 ///< Near flag for Carmack compression.
#define FAR  0xA8 ///< Far flag for Carmack compression.

#pragma mark -

int rlew_expand(word *const source, word *const destination, const uint length, const word rlew_tag) {
	word *read = source, *write = destination, *end = destination + length; // read- and write heads of the algorightm
	word current_word;
	uint count = 0, i = 0;
	
	if (source == NULL || destination == NULL) {
		fprintf(stderr, "\t\tDecompression error: source or destination buffers are NULL.\n");
		return NULL_POINTERS;
	}
	
	while (write < end) {
		if ((current_word = *(read++)) == rlew_tag) {
			count = *(read++);
			for (i = 0; i < count; ++i) {
				*(write++) = *read;
			}
			++read;
		} else {
			*(write++) = current_word;
		}
	}
	return NO_ERROR;
}

int carmack_expand(word *const source, word *const destination, word length) {
	// the read-, write- and copy pointers are byte pointers for smaller iteration steps
	byte *read = (byte *)source, *write = (byte *)destination, *copy; // read- and write heads of the algorithm (as bytes)
	byte flag;
	uint offset = 0, count = 0, i = 0;
	
	#define COPY_WORD {*(write++) = *(copy++); *(write++) = *(copy++); --length;} ///< This is a workaround for lack of nested functions.
	
	if (source == NULL || destination == NULL) {
		DEBUG_PRINT("\t\tDecompression error: source or destination buffers are NULL.\n")
		return NULL_POINTERS;
	}
	DEBUG_PRINT("\nBeginning Carmack-expansion to %i words length...\n", length)
	while (length > 0) {
		count = *(read++);
		flag  = *(read++);
		DEBUG_PRINT("\tLength: %x count: %i, flag: %x; ", length, count, flag);
		if (flag == NEAR && count != 0) {
			DEBUG_PRINT("Near pointer")
			offset = *(read++);
			copy = write - 2 * offset; // *2 because offset is in bytes
			for (i = 0; i < count; ++i) {
				COPY_WORD
			}
		} else if (flag == FAR && count != 0) {
			DEBUG_PRINT("Far pointer")
			offset = *((word *)read);
			read += 2;
			copy = (byte *)destination + 2 * offset;
			for (i = 0; i < offset; ++i) {
				COPY_WORD
			}
		} else if ((flag == NEAR || flag == FAR) && count == 0) {
			DEBUG_PRINT("Exception")
			*(write++) = *(read++);
			*(write++) = flag;
			--length;
		} else {
			// Don't move the read head, it's already in place.
			*(write++) = count;
			*(write++) = flag;
			--length;
		}
		DEBUG_PRINT("\n")
	}
	
	return NO_ERROR;
	#undef COPY_WORD
}
