#include "globals.h"
#include <string.h>

uint debug_level = 0;

char extensions[GAME_VERSIONS][4] = {WL1_S, WL3_S, WL6_S};
char extension[4] = {'\0', '\0', '\0', '\0'};
uint current_game_version = 0;

/*----------------------------------------------------------------------------*/

void change_extension(char *restrict file_name, const char *restrict extension) {
	int n = (int)strlen(file_name); // strlen does not count the terminating `\0`.
	for (int i = 0; i < 3; ++i) {
		file_name[n - 3 + i] = extension[i];
	}
}

inline void debug_print(int level, char *format, ...) {
	if (level <= debug_level) {
		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
	}
}

