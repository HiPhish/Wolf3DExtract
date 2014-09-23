#ifndef Wolf3DExtract_compression_h
#define Wolf3DExtract_compression_h

#include "../globals.h"

//Error codes for the compression routines.
#define NO_ERROR      0 ///< No error, everything fine.
#define NULL_POINTERS 1 ///< One or more pointers are invalid.
#define MALLOC_FAIL   2 ///< Failed to allocate memory to a pointer.

/**
 *  Expands an RLEW-compressed sequence into a pre-allocated buffer.
 *
 *  @param source      Pointer to the compressed source sequence.
 *  @param destination Pointer to the buffer to be filled with the expanded sequence.
 *  @param length      Length of the expaneded sequence in *words*.
 *  @param rlew_tag    Word used to identify the RLEW compressed bytes.
 *
 *  @return Returns `0` for no error, otherwise an error code.
 */
int rlew_expand(word * const source, word *const destination, const uint length, const word rlew_tag);

/**
 *  Expands a Carmack-compressed sequence into a pre-allocated buffer.
 *
 *  @param source      Pointer to the compressed source sequence.
 *  @param destination Pointer to the buffer to be filled with the expanded sequence.
 *  @param length      Length of the expaneded sequence in *words*.
 *
 *  @return Returns `0` for no error, otherwise an error code.
 */
int carmack_expand(word *const source, word *const destination, word length);

#endif // Wolf3DExtract_compression_h
