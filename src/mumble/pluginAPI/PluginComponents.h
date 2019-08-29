/**
 * This header file contains definitions of types and other components used in Mumble's plugin system
 */

#ifndef MUMBLE_PLUGINCOMPONENT_H_
#define MUMBLE_PLUGINCOMPONENT_H_

#include <stdint.h>

// Define an enum for error codes
enum Status {
	OK=0,
	ERROR
};

struct Version {
	int32_t major;
	int32_t minor;
	int32_t patch;
};

struct MumbleFunctions {
};


#endif
