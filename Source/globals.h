#ifndef Wolf3DExtract_globals_h
#define Wolf3DExtract_globals_h

#include <stdint.h>

// Map extensions to strings
#define WL1_S "WL1" ///< Shareware version.
#define WL3_S "WL3" ///< Older three-episode registered version.
#define WL6_S "WL6" ///< Newer six-episode registered version.

// Map extensions to indices
#define WL1_I 0 ///< Shareware version index.
#define WL3_I 1 ///< Older three-episode registered version index.
#define WL6_I 2 ///< Newer six-episode registered version index.

#define GAME_VERSIONS 3 ///< Number of supported game versions.

typedef unsigned int uint; ///< Shorthand writing.

typedef uint8_t  byte; ///< 8-bit byte.
typedef uint16_t word; ///< 2-byte word.

/** Array listing the possible extensions used by the game's data. */
extern char extensions[GAME_VERSIONS][4];

#pragma mark -

extern char extension[4]; ///< File extension as identified by the first call.

void change_extension(char *restrict file_name, const char *restrict extension);

/** Current version of the game, idetified by the index `WLn_I`. */
extern uint current_game_version;

#pragma mark Diagnostics macros

extern uint debug_level; ///< A debug level of 0 is standard and means no debug messages.

/**
 *  Prints a message to stderr.
 *
 *  The message will only be printed if the level of the message is less or equal to the debug level.
 */
#define DEBUG_PRINT(l, ...) \
	if (l <= debug_level) {fprintf(stderr, __VA_ARGS__);}

#endif // Wolf3DExtract_globals_h

