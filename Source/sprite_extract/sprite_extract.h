#ifndef Wolf3DExtract_sprite_extract_h
#define Wolf3DExtract_sprite_extract_h

#include <stdlib.h>
#include "../globals.h"

// Error codes
#define SE_FILE_NOT_FOUND 1; ///< Error opening file.
#define SE_MALLOC_FAIL    2; ///< Error trying to allocate memory.

size_t extract_sprite(byte **buffer, uint index);

#endif // Wolf3DExtract_sprite_extract_h

