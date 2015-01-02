#ifndef Wolf3DExtract_sound_extract_h
#define Wolf3DExtract_sound_extract_h

#include <stdlib.h>
#include "../globals.h"


/*-[ TYPE DEFINTIONS ]--------------------------------------------------------*/

/** Format of the sound file. */
typedef int sound_format;


/*-[ CONSTANTS ]--------------------------------------------------------------*/

//Sound effect format
#define PC_SPEAKER    0 ///< PC speaker sound format.
#define ADLIB_SOUND   1 ///< AdLib sound format.
#define DIGI_SOUND    2 ///< Digitised sound format.
#define SOUND_FORMATS 3 ///< Number of possible sound effect formats.


/*-[ FUNCTION DECLARATIONS ]--------------------------------------------------*/

/** Extracts a sound effect into a buffer.
 *
 *  @param buffer        Pointer to the buffer to store the sound effect into.
 *  @param magic_number  Magic number of a sound effect.
 *  @param format        Format of the sound file.
 *
 *  @return  Size of the sound effect in bytes, or 0 on error.
 *
 *  The buffer should not be allocated, it will be allocated by the function. It
 *  needs to be freed manually later.
 */
size_t extract_sound(byte **buffer, uint magic_number, sound_format format);

/** Extracts a music track into a buffer.
 *
 *  @param buffer        Pointer to the buffer to store the music track into.
 *  @param magic_number  Magic number of a sound effect.
 * 
 *  @return  Size of the sound effect in bytes, or 0 on error.
 * 
 *  The buffer should not be allocated, it will be allocated by the function. It
 *  needs to be freed manually later.
 */
size_t extract_music(byte **buffer, uint magic_number);

#endif // Wolf3DExtract_sound_extract_h

