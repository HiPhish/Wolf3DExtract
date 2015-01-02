#ifndef Wolf3DExtract_globals_h
#define Wolf3DExtract_globals_h

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>


/*-[ CONSTANTS ]--------------------------------------------------------------*/

/* Map extensions to strings */
#define WL1_S "WL1" ///< Shareware version.
#define WL3_S "WL3" ///< Older three-episode registered version.
#define WL6_S "WL6" ///< Newer six-episode registered version.

/* Map extensions to indices */
#define WL1_I 0 ///< Shareware version index.
#define WL3_I 1 ///< Older three-episode registered version index.
#define WL6_I 2 ///< Newer six-episode registered version index.

#define GAME_VERSIONS 3 ///< Number of supported game versions.


/*-[ TYPE DEFINITIONS ]-------------------------------------------------------*/

typedef unsigned int uint; ///< Shorthand writing for unsigned integers.

/* The terms "byte", "word" and "dword" are defined in ralation to the original
   hardware. */

typedef uint8_t   byte; ///< 8-bit byte.
typedef uint16_t  word; ///< 2-byte word.
typedef uint32_t dword; ///< 4-byte double-word.

/** Array listing the possible extensions used by the game's data. */
extern char extensions[GAME_VERSIONS][4];

#pragma mark -


/*-[ GLOBAL VARIABLES ]-------------------------------------------------------*/

/** File extension as identified by the first call.
 *
 *  The file extension is always three characters wide, plus one character to
 *  terminate the string.
 */
extern char extension[4];

/** Current version of the game, idetified by the index `WLn_I`. */
extern uint current_game_version;

/** Debug level for filtering which messages to display.
 *
 *  A debug level of 0 is standard and means no debug messages. Eventually this
 *  should be replaced with flags that can be bitwise ORed together to allow
 *  for more precise control over the types of messages.
 */
extern uint debug_level;


/*-[ GLOBAL FUNCTIONS ]-------------------------------------------------------*/

/** Change which extension is currently used for files.
 *  
 *  @param file_name
 *  @param extension
 *
 *  asdf.
 */
void change_extension(char *restrict file_name, const char *restrict extension);

/**
 * Prints a message to the standard error.
 *
 * @param level   If the value is higher than the debug level of the program
 *                nothing is printed.
 * @param format  Formatted string to print.
 *
 * The message will only be printed if the level of the message is less or equal to the debug level.
 */
void debug_print(int level, char *format, ...);

#pragma mark Diagnostics macros

/*----------------------------------------------------------------------------*/

// Deprecated in favour of the above, should be removed.
#define DEBUG_PRINT(l, ...) \
	if (l <= debug_level) {fprintf(stderr, __VA_ARGS__);}

#endif // Wolf3DExtract_globals_h

