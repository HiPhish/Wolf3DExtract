#ifndef Wolf3DExtract_pic_extract_h
#define Wolf3DExtract_pic_extract_h

#include "../globals.h"
#include <stdlib.h>


//Error codes for picture extraction.
//These codes are returned by the extraction methods; they can then be mapped by another function to handle the error appropriately instead of just abort.
#define NO_ERROR       0 ///< No error, everything fine.
#define FILE_NOT_FOUND 1 ///< Cannot find file to load.
#define MALLOC_FAIL    2 ///< Failed to allocate memory to a pointer.
#define LOAD_FAIL      3 ///< Could not load data from file.
#define PIC_ERRORS     4 ///< Number of map extraction errors.

/**
 *  Structure describing a bitmap image of Wolfenstein 3D.
 *
 *  The array of texel bytes is not stored in the struct, instead it's pointed at. The array
 *  size varies from pic to pic, so it can't be stored inside the struct.
 */
struct picture {
	int16_t width;  ///< Width of the bitmap image.
	int16_t height; ///< Height of the bitmap image.
	byte *textels;  ///< Array of bitmap texels.
};

/**
 *  Extracts the vgagraph chunk offsets into a buffer.
 *
 *  @param buffer Pointer to an array of signed 32-bit integers.
 *
 *  @return Size of the array, 0 if an error occured.
 */
size_t extract_pic_offsets(int32_t **buffer);

/**
 *  Extracts the picture table into a buffer.
 *
 *  @param buffer Pointer to an array of words.
 *
 *  @return Size of the array, 0 if an error occured.
 */
size_t extract_pic_table(word **buffer);

/**
 *  Extract a bitmap picture (not sprite or texture) into a buffer.
 *
 *  @param buffer       Pointer to a pointer for the resulting picture.
 *  @param magic_number Magic number of the picture.
 *
 *  @return Amount of allocated memory, 0 of an error occurs, size of a picture struct otherwise.
 *
 *  The buffer does not have to be allocated, it will be freed and re-allocated automatically.
 */
size_t extract_pic(struct picture **buffer, uint magic_number);

#endif //Wolf3DExtract_pic_extract_h

