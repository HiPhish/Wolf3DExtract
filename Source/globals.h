#ifndef Wolf3DExtract_globals_h
#define Wolf3DExtract_globals_h

#include <stdint.h>

#define WL1 "WL1" ///< Shareware version.
#define WL3 "WL3" ///< Older three-episode registered version.
#define WL6 "WL6" ///< Newer six-episode registered version.

#define NUMBER_OF_EXTENSIONS 3 ///< Number of possible extensions

typedef unsigned int uint; ///< Shorthand writing.

typedef uint8_t  byte; ///< 8-bit byte.
typedef uint16_t word; ///< 2-byte word.

/**
 *  Array listing the possible extensions used by the game's data.
 */
char extensions[NUMBER_OF_EXTENSIONS][4];

#pragma mark -

char extension[4]; ///< File extension as identified by the first call.

#pragma mark Diagnostics macros

#ifdef DEBUG
#define DEBUG_PRINT(...) \
	fprintf(stderr, __VA_ARGS__);
#else
#define	DEBUG_PRINT(...)
#endif

#endif // Wolf3DExtract_globals_h
