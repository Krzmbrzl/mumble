/**
 * This header file contains definitions of types and other components used in Mumble's plugin system
 */

#ifndef MUMBLE_PLUGINCOMPONENT_H_
#define MUMBLE_PLUGINCOMPONENT_H_

#include <stdint.h>

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


struct MumbleUser {
	const char *name;
	uint32_t id;
};

struct MumbleChannel {
	const char *name;
	int32_t id;
};


typedef enum TalkingState TalkingState_t;
typedef struct Version Version_t;
typedef struct MumbleUser MumbleUser_t;
typedef struct MumbleChannel MumbleChannel_t;

typedef int32_t MumbleConnection_t


struct MumbleAPI {
	// -------- Memory (de-)allocation --------
	
	/// Allocates memory for a MumbleUser. Once the user struct is no longer needed, it has to be freed by calling
	/// freeMumbleUser.
	///
	/// @returns A pointer to the allocated struct.
	MumbleUser_t* (*allocateMumbleUser)();

	/// Frees a MumbleUser that has been allocated via allocateMumbleUser.
	///
	/// @params user A pointer to the user struct that should be freed
	void (*freeMumbleUser)(MumbleUser_t *user);


	/// Allocates memory for a MumbleChannel. Once the channel struct is no longer needed, it has to be freed by calling
	/// freeMumbleChannel.
	///
	/// @returns A pointer to the allocated struct.
	MumbleChannel_t* (*allocateMumbleChannel)();

	/// Frees a MumbleChannel that has been allocated via allocateMumbleChannel.
	///
	/// @param channel A pointer to the channel struct that should get freed
	void (*freeMumbleChannel)(MumbleChannel_t *channel);

	

	
	// -------- Getter functions --------

	/// Fills in the connection ID of the server the user is currently active on (the user's audio output is directed at).
	///
	/// @param[out] connection A pointer to the memory location the ID should be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then it is valid to access the
	/// 	value of the provided pointer
	uint32_t getActiveServerConnection(MumbleConnection_t *connection);

	/// Fills in the information about the local user.
	///
	/// @param connection A reference to which server-connection should be considered.
	/// @param[out] user The user struct allocated via allocateMumbleUser whose fields will be set by this function.
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the fields of the
	/// 	passed struct may be accessed.
	uint32_t (*getLocalUser)(MumbleConnection_t connection, MumbleUser_t *user);

	/// Fills in the information about the user with the provided ID, if such a user exists.
	///
	/// @param userID The respective user's ID
	/// @param connection A reference to which server-connection should be considered.
	/// @param[out] user The user struct allocated via allocateMumbleUser whose fields will be set by this function.
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the fields of the
	/// 	passed struct may be accessed.
	uint32_t (*getUser)(MumbleConnection_t connection, uint32_t userID, MumbleUser_t *user);

	///	Fills in the information about the channel with the given ID, if such a channel exists.
	///
	/// @param connection A reference to which server-connection should be considered.
	/// @param channelID The respective channel's ID
	/// @param[out] channel The channel struct allocated via allocateMumbleChannel whose fields will be set by this function.
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the fields of the passed
	/// 	struct may be accessed.
	uint32_t (*getChannel)(MumbleConnection_t connection, int32_t channelID, MumbleChannel_t *channel);

	/// Fills in the amount of clients that are currently connected to the server.
	///
	/// @param connection A reference to which server-connection should be considered.
	/// @param[out] A pointer to the memory the amount shall be written
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then it is valid to access the
	/// 	value of the provided pointer.
	uint32_t (*getAmountOfConnectedClients)(MumbleConnection_t connection, size_t *amount);



	// -------- Find functions --------
	
	/// Fills in the information about a user with the specified name, if such a user exists.
	///
	/// @param connection A reference to which server-connection should be considered.
	/// @param userName The respective user's name
	/// @param[out] user The user struct allocated via allocateMumbleUser whose fields will be set by this function
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the fields of the passed
	/// 	struct may be accessed.
	uint32_t (*findUserByName)(MumbleConnection_t connection, const char *userName, MumbleUser_t *user);

	/// Fills in the information about a channel with the specified name, if such a channel exists.
	///
	/// @param connection A reference to which server-connection should be considered.
	/// @param channelName The respective channel's name
	/// @param[out] channel The channel struct allocated via allocateMumbleChannel whose fields will be set by this function
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the fields of the passed
	/// 	struct may be accessed.
	uint32_t (*findChannelByName)(MumbleConnection_t connection, const char *channelName, MumbleChannel_t *channel);
};


#endif
