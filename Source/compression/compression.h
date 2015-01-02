#ifndef Wolf3DExtract_compression_h
#define Wolf3DExtract_compression_h

#include "../globals.h"


/*-[ ERROR CODES FOR COMPRESSION ROUTINES ]-----------------------------------*/

#define NO_ERROR      0 ///< No error, everything fine.
#define NULL_POINTERS 1 ///< One or more pointers are invalid.
#define MALLOC_FAIL   2 ///< Failed to allocate memory to a pointer.


/*-[ TYPE DEFINITIONS ]-------------------------------------------------------*/

/** A node in a Huffman tree. */
struct huffman_node {
	word node_0; ///< Index of left node, take when bit is 0.
	word node_1; ///< Index of right node, take when bit is 1.
};


/*-[ FUNCTION DECLARATIONS ]--------------------------------------------------*/

/** Expands an RLEW-compressed sequence into a pre-allocated buffer.
 *
 *  @param source       Pointer to the compressed source sequence.
 *  @param destination  Pointer to the buffer to be filled with the expanded
 *                      sequence.
 *  @param length       Length of the expaneded sequence in *words*.
 *  @param rlew_tag     Word used to identify the RLEW compressed bytes.
 * 
 *  @return  Returns `0` for no error, otherwise an error code.
 */
int rlew_expand(word * const source, word *const destination, const word length, const word rlew_tag);

/** Expands a Carmack-compressed sequence into a pre-allocated buffer.
 *
 *  @param source       Pointer to the compressed source sequence.
 *  @param destination  Pointer to the buffer to be filled with the expanded
 *                      sequence.
 *  @param length       Length of the expaneded sequence in *words*.
 * 
 *  @return  Returns `0` for no error, otherwise an error code.
 */
int carmack_expand(word *const source, word *const destination, word length);

/** Expands a Huffman-compressed sequence of bytes into a pre-allocated buffer.
 *
 *  @param source       Pointer to the compressed source sequence.
 *  @param destination  Pointer to the buffer to be filled with the expanded
 *                      sequence.
 *  @param length       Length of the expaneded sequence in *bytes*.
 *  @param tree         Pointer to root of the Huffman tree for decompression.
 * 
 *  @return  Returns `0` for no error, otherwise an error code.
 */
int huffman_expand(byte *const source, byte *destination, const int32_t length, struct huffman_node *tree);

#endif // Wolf3DExtract_compression_h

