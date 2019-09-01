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


struct MumbleUser {
	char *name;
	uint32_t id;
};

struct MumbleChannel {
	char *name;
	int32_t id;
};


struct MumbleAPI {
	// -------- Memory management --------
	
	/// Frees the provided pointer.
	///
	/// @param pointer The pointer to free - may be NULL
	void (*freeMemory)(void *pointer);

	
	// -------- Getter functions --------

	/// Gets the local user. The returned pointer must be freed by using the freeMemory function.
	///
	/// @returns A pointer to the MumbleUser describing the local user.
	const MumbleUser* (*getLocalUser)();

	/// Gets the user with the respective ID. The returned pointer must be freed using the freeMemory function.
	///
	/// @param userID The ID of the user that should be obtained.
	/// @returns A pointer to the respective user or NULL if no such user could be found
	const MumbleUser* (*getUser)(uint32_t userID);

	/// Gets the channel with the given ID. The returned pointer must be freed using the freeMemory function.
	///
	/// @param channelID The Id of the channel to retrieve
	/// @returns A pointer to the respective channel or NULL if no such channel could be found
	const MumbleChannel* (getChannel)(int32_t channelID);


	// -------- Find functions --------
	
	/// Finds a user with the specified name. The returned pointer must be freed by using the freeMemory function.
	///
	/// @param userName The name of the user to search for (case-sensitive)
	/// @returns A pointer to the respective user or NULL if no such user could be found
	MumbleUser* (*findUserByName)(const char *userName);

	/// Finds a channel with the specified name. The returned pointer must be freed by using the freeMemory function.
	///
	/// @param channelName The name of the user to search for (case-sensitive)
	/// @returns A pointer to the respective channel or NULL if no such channel could be found
	MumbleChannel* (*findChannel)(const char *channelName);
};


#endif
