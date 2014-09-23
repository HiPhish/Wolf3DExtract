#ifndef Wolf3DExtract_map_extract_h
#define Wolf3DExtract_map_extract_h

#include "../globals.h"

#pragma mark Constants

//The three planes (maps) a level is made of.
#define MAP_ARCHITECTURE 0 ///< Floors and walls.
#define MAP_OBJECTS      1 ///< Actors and decoration obkjects.
#define MAP_LOGIC        2 ///< Waypoints and triggers.
#define MAP_PLANES       3 ///< Number of maps that form a level.

#define MAX_LEVELS     100 ///< Maximum number of levels in the game.
#define EPISODE_LEVELS  10 ///< Number of levels per episode.

/**
 *  Error codes for data extraction.
 *  
 *  These codes are returned by the extraction methods; they can then be mapped by another function to handle the error appropriately instead of just
 *  aborting.
 */
enum map_error {
	NO_ERROR      , ///< No error, everything fine.
	FILE_NOT_FOUND, ///< Cannot find file to load.
	MALLOC_FAIL   , ///< Failed to allocate memory to a pointer.
	LOAD_FAIL     , ///< Could not load data from file.
	
	MAP_ERRORS      ///< Number of map extraction errors.
};

#pragma mark Structures

/**
 *  Structure describing how to find the individual levels in the GAMEAPS file.
 */
struct level_atlas {
	word    rlew_tag;                  ///< Signature for RLEW decompression.
	int32_t header_offset[MAX_LEVELS]; ///< Offsets to the individual level headers.
	//byte  tile_info;                 ///< Unused in Wolfenstein 3D, leftover from earlier games.
};

/**
 *  Header of a level holding general information about a certain level.
 *
 *  The levels are too large to be all loaded into memory at the same time, instead these headers can be used to
 *  find the individual maps and assemble the level.
 */
struct level_header {
	int32_t map_offest[MAP_PLANES]; ///< Offsets of the maps, relative to the beginning of the file.
	word    cc_length[MAP_PLANES];  ///< Carmack-compressed length of the maps.
	word    width;                  ///< Width of the level.
	word    height;                 ///< Height of the level.
	byte    name[16];               ///< Name of the level
};

/**
 *  Extracts the level atlas into a given buffer.
 *
 *  @param buffer Buffer to extract into.
 *
 *  @return Size of the allocated buffer if everything went right, zero otherwise.
 *
 *  The buffer does not need to be pre-allocated, the function will free its memory and allocate enough space. If an error occurs the buffer will not get
 *  changed.
 */
size_t extract_level_atlas(struct level_atlas **buffer);

/**
 *  Extracts a level's header into a given buffer.
 *
 *  @param buffer  Buffer to extract into.
 *  @param episode Episode of the level to load.
 *  @param level   Level within the episode.
 *
 *  @return Size of the allocated buffer if everything went right, zero otherwise.
 *
 *  The buffer does not need to be pre-allocated, the function will free its memory and allocate enough space. If an error occurs the buffer will not get
 *  changed.
 */
size_t extract_level_header(struct level_header **buffer, uint episode, uint level);

/**
 *  Extracts a level's map into a given buffer.
 *
 *  @param buffer  Buffer to extract into.
 *  @param episode Episode of the level to load.
 *  @param level   Level within the episode.
 *  @param map     Map of the level to load.
 *
 *  @return Size of the allocated buffer if everything went right, zero otherwise.
 *
 *  The buffer does not need to be pre-allocated, the function will free its memory and allocate enough space. If an error occurs the buffer will not get
 *  changed.
 */
size_t extract_map(word **buffer, uint episode, uint level, uint map);

#endif // Wolf3DExtract_map_extract_h
