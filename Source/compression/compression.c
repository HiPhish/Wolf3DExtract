#include <stdio.h>
#include <stdlib.h>
#include "compression.h"

#define NEAR 0xA7 ///< Near flag for Carmack compression.
#define FAR  0xA8 ///< Far flag for Carmack compression.
#define ROOT 254  ///< Root node index for Huffman compression.

/** Check if source and destination pointer are not NULL. */
#define CHECK_FOR_NULL_REFERENCES \
	if (source == NULL || destination == NULL) {\
		fprintf(stderr, "\t\tDecompression error: source or destination buffers are NULL.\n");\
		return NULL_POINTERS;\
	}

#pragma mark -

int rlew_expand(word *const source, word *const destination, const word length, const word rlew_tag) {
	CHECK_FOR_NULL_REFERENCES
	
	word *read = source, *write = destination, *end = destination + length; // read- and write heads of the algorightm
	word current_word;
	uint count = 0, i = 0;
	
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
	CHECK_FOR_NULL_REFERENCES

	// the read-, write- and copy pointers are byte pointers for smaller iteration steps
	byte *read = (byte *)source, *write = (byte *)destination, *copy; // read- and write heads of the algorithm (as bytes)
	byte flag;
	word offset = 0, count = 0, i = 0;
	// compressed blocks have the following form: (count, flag, offset, [offset]); offset is a byte for near pointers, a word for far pointers

	#define COPY_WORD {*(write++) = *(copy++); *(write++) = *(copy++); --length;} ///< This is a workaround for lack of nested functions.
	
	DEBUG_PRINT(1, "\nBeginning Carmack-expansion to %i words length...\n", length)
	while (length > 0) {
		count = *(read++);
		flag  = *(read++);
		DEBUG_PRINT(2, "\tLength: %x count: %i, flag: %x; ", length, count, flag);
		if (flag == NEAR && count != 0) {
			DEBUG_PRINT(2, "Near pointer")
			offset = *(read++);
			copy = write - 2 * offset; // *2 because write is a byte pointer and we want to copy words
			for (i = 0; i < count; ++i) {
				COPY_WORD
			}
		} else if (flag == FAR && count != 0) {
			DEBUG_PRINT(2, "Far pointer")
			offset = *((word *)read);
			read += 2;
			copy = (byte *)(destination + offset); // destination is a word pointer
			for (i = 0; i < count; ++i) {
				COPY_WORD
			}
		} else if ((flag == NEAR || flag == FAR) && count == 0) {
			DEBUG_PRINT(2, "Exception")
			*(write++) = *(read++);
			*(write++) = flag;
			--length;
		} else {
			// Don't move the read head, it's already in place.
			*(write++) = count;
			*(write++) = flag;
			--length;
		}
		DEBUG_PRINT(1, "\n")
	}
	
	return NO_ERROR;
	#undef COPY_WORD
}


int huffman_expand(byte *const source, byte *destination, int32_t length, struct huffman_node *tree) {
	CHECK_FOR_NULL_REFERENCES

	byte *read  = source, *write = destination;
	byte mask   = 0x01;
	byte input  = *(read++);

	word node_value;
	struct huffman_node *node = &tree[ROOT];
	int32_t bytes_written = 0;

	DEBUG_PRINT(1, "Starting Huffman decompression.\n")
	while (1) {
		if ((input & mask) == 0) {
			node_value = node->node_0;
			DEBUG_PRINT(2, "\tRead bit 0, switching to left node.\n")
		} else {
			node_value = node->node_1;
			DEBUG_PRINT(2, "\tRead bit 1, switching to right node.\n")
		}

		if (mask == 0x80) { // 0x80 = 0b10000000
			DEBUG_PRINT(2, "\tMasking byte has reached end, resetting and reading next byte.\n")
			input = *(read++);
			mask  = 0x01;
		} else {
			DEBUG_PRINT(2, "\tAdvancing masking byte\n")
			mask <<= 1;
		}
		if (node_value <= 0xFF) { // 0xFF = 0b11111111 = 255
			DEBUG_PRINT(2, "\tLeaf node reached, printing word %x.\n", node_value)
			*(write++) = (byte)node_value;
			node = &tree[ROOT];
			if ((++bytes_written) == length) {
				DEBUG_PRINT(2, "End of compressed sequence. ")
				break;
			}
		} else {
			DEBUG_PRINT(2, "\tSwitching to node %i.\n", node_value - 256)
			node = tree + node_value - 256;
		}
	}
	DEBUG_PRINT(2, "Finished Huffman decompression.\n")

	return 0;
}

