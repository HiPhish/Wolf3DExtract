#ifndef Wolf3DExtract_sprite_extract_h
#define Wolf3DExtract_sprite_extract_h

#include <stdlib.h>
#include "../globals.h"

/** Extract a texture into a memory buffer.
 *
 *  @param buffer        Pointer to a sequence of bytes.
 *  @param magic_number  Index number of the texture.
 * 
 *  @return  Size of the texture (64*64) on success, 0 otherwise.
 */
size_t extract_texture(byte **buffer, uint magic_number);

/** Extract a sprite into a memory buffer.
 *
 *  @param buffer        Pointer to a sequence of bytes.
 *  @param magic_number  Index number of the sprite.
 * 
 *  @return  Size of the sprite (64*64) on success, 0 otherwise.
 */
size_t extract_sprite(byte **buffer, uint magic_number);

#endif // Wolf3DExtract_sprite_extract_h

