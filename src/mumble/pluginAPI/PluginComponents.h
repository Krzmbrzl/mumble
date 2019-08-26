/**
 * This header file contains definitions of types and other components used in Mumble's plugin system
 */

#ifndef MUMBLE_PLUGINCOMPONENT_H_
#define MUMBLE_PLUGINCOMPONENT_H_

// Define an enum for error codes
enum Status {
	OK=0,
	ERROR
};

struct Version {
	int major;
	int minor;
	int patch;
};

struct MumbleFunctions {
};


#endif
