/**
 * This header file contains definitions of types and other components used in Mumble's plugin system
 */

#ifndef MUMBLE_PLUGINCOMPONENT_H_
#define MUMBLE_PLUGINCOMPONENT_H_

#include <stdint.h>
#include <stddef.h>

#define STATUS_OK 0

enum TalkingState {
	INVALID=-1,
	PASSIVE=0,
	TALKING,
	WHISPERING,
	SHOUTING
};

struct Version {
	int32_t major;
	int32_t minor;
	int32_t patch;
};


typedef enum TalkingState TalkingState_t;
typedef struct Version Version_t;
typedef int32_t MumbleConnection_t;
typedef uint32_t MumbleUserID_t;
typedef int32_t MumbleChannelID_t;
typedef int32_t error_t;


struct MumbleAPI {
	// -------- Memory management --------
	
	/// Frees the given pointer.
	///
	/// @param pointer The pointer to free
	/// @returns The error code. If everything went well, STATUS_OK will be returned.
	error_t (*freeMemory)(void *pointer);


	
	// -------- Getter functions --------

	/// Gets the connection ID of the server the user is currently active on (the user's audio output is directed at).
	///
	/// @param[out] connection A pointer to the memory location the ID should be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then it is valid to access the
	/// 	value of the provided pointer
	error_t (*getActiveServerConnection)(MumbleConnection_t *connection);

	/// Fills in the information about the local user.
	///
	/// @param connection The ID of the server-connection to use as a context
	/// @param[out] userID A pointer to the memory the user's ID shall be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the fields of the
	/// 	passed struct may be accessed.
	error_t (*getLocalUserID)(MumbleConnection_t connection, MumbleUserID_t *userID);

	/// Fills in the information about the given user's name.
	///
	/// @param connection The ID of the server-connection to use as a context
	/// @param userID The user's ID whose name should be obtained
	/// @param[out] A pointer to where the pointer to the allocated string (C-ecoded) should be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer
	/// 	may be accessed.
	error_t (*getUserName)(MumbleConnection_t connection, MumbleUserID_t userID, const char **userName);



	// -------- Find functions --------
	
	/// Fills in the information about a user with the specified name, if such a user exists.
	///
	/// @param connection The ID of the server-connection to use as a context
	/// @param userName The respective user's name
	/// @param[out] userID A pointer to the memory the user's ID shall be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the fields of the passed
	/// 	struct may be accessed.
	error_t (*findUserByName)(MumbleConnection_t connection, const char *userName, MumbleUserID_t *userID);

	/// Fills in the information about a channel with the specified name, if such a channel exists.
	///
	/// @param connection The ID of the server-connection to use as a context
	/// @param channelName The respective channel's name
	/// @param[out] channelID A pointer to the memory the channel's ID shall be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the fields of the passed
	/// 	struct may be accessed.
	error_t (*findChannelByName)(MumbleConnection_t connection, const char *channelName, MumbleChannelID_t *channelID);
};


#endif
