#include <string.h>

enum class Level {
	// Log of debugging operations.
	DEBUG,
	// Log of normal operations, to be used in release build.
	INFO,
	// Log of an event thay may indicates a problem.
	WARN,
	// Log of an event that indicates an error.
	ERROR,
	// Log of an even that indicates an unrecoverable error.
	CRITICAL,
	//Level used to disable all logging.
	NONE,
	// An unknown severity level.
	UNKNOWN
};

/**
 * Get the name of a Level value.
 * @param level The Level to get the name of.
 * @return Returns the name of the Level. If the level is not recognized, returns "UNKNOWN".
 */
std::string convertLevelToName(Level level);

/**
 * Get the Level corresponding to a Level name.
 * @param name The name corresponding to the desired Level value.
 * @return The @c Level corresponding to the specified name. If the @c name is not recognized,
 * returns @c Level::UNKNOWN.
 */
Level convertNameToLevel(const std::string& name);
