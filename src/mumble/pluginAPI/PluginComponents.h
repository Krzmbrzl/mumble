/**
 * This header file contains definitions of types and other components used in Mumble's plugin system
 */

#ifndef MUMBLE_PLUGINCOMPONENT_H_
#define MUMBLE_PLUGINCOMPONENT_H_

#include <stdint.h>

#define STATUS_OK 0

struct Version {
	int32_t major;
	int32_t minor;
	int32_t patch;
};


struct MumbleUser {
	const char *name;
	uint32_t id;
};

struct MumbleChannel {
	const char *name;
	int32_t id;
};

typedef struct Version Version_t;
typedef struct MumbleUser MumbleUser_t;
typedef struct MumbleChannel MumbleChannel_t;


struct MumbleAPI {
	// -------- Memory (de-)allocation --------
	
	MumbleUser_t* (*allocateMumbleUser)();

	void (*freeMumbleUser)(MumbleUser_t *user);

	MumbleChannel_t* (*allocateMumbleChannel)();

	void (*freeMumbleChannel)(MumbleChannel_t *channel);

	

	
	// -------- Getter functions --------

	/// Gets the local user. The returned pointer must be freed by using the freeMemory function.
	///
	/// @returns A pointer to the MumbleUser describing the local user.
	const MumbleUser_t* (*getLocalUser)();

	/// Gets the user with the respective ID. The returned pointer must be freed using the freeMemory function.
	///
	/// @param userID The ID of the user that should be obtained.
	/// @returns A pointer to the respective user or NULL if no such user could be found
	const MumbleUser_t* (*getUser)(uint32_t userID);

	/// Gets the channel with the given ID. The returned pointer must be freed using the freeMemory function.
	///
	/// @param channelID The Id of the channel to retrieve
	/// @returns A pointer to the respective channel or NULL if no such channel could be found
	const MumbleChannel_t* (*getChannel)(int32_t channelID);


	// -------- Find functions --------
	
	/// Finds a user with the specified name. The returned pointer must be freed by using the freeMemory function.
	///
	/// @param userName The name of the user to search for (case-sensitive)
	/// @returns A pointer to the respective user or NULL if no such user could be found
	MumbleUser_t* (*findUserByName)(const char *userName);

	/// Finds a channel with the specified name. The returned pointer must be freed by using the freeMemory function.
	///
	/// @param channelName The name of the user to search for (case-sensitive)
	/// @returns A pointer to the respective channel or NULL if no such channel could be found
	MumbleChannel_t* (*findChannel)(const char *channelName);
};


#endif
