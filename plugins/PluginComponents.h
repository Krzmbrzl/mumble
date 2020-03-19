/**
 * This header file contains definitions of types and other components used in Mumble's plugin system
 */

#ifndef MUMBLE_PLUGINCOMPONENT_H_
#define MUMBLE_PLUGINCOMPONENT_H_

#include <stdint.h>
#include <stddef.h>
#include <string>

#ifdef QT_VERSION
	#include <QString>
#endif

#if defined(_MSC_VER)
	#define PLUGIN_CALLING_CONVENTION __cdecl
#elif defined(__MINGW32__)
	#define PLUGIN_CALLING_CONVENTION __attribute__((cdecl))
#else
	#define PLUGIN_CALLING_CONVENTION
#endif


#define STATUS_OK EC_OK
#define VERSION_UNKNOWN Version({0,0,0})

enum PluginFeature {
	/// None of the below
	FEATURE_NONE = 0,
	/// The plugin provides positional data from a game
	FEATURE_POSITIONAL = 1 << 0,
	/// The plugin modifies the input/output audio itself
	FEATURE_AUDIO = 1 << 1
};

enum TalkingState {
	INVALID=-1,
	PASSIVE=0,
	TALKING,
	WHISPERING,
	SHOUTING
};

enum TransmissionMode {
	TM_CONTINOUS,
	TM_VOICE_ACTIVATION,
	TM_PUSH_TO_TALK
};

enum ErrorCode {
	EC_GENERIC_ERROR = -1,
	EC_OK = 0,
	EC_POINTER_NOT_FOUND,
	EC_NO_ACTIVE_CONNECTION,
	EC_USER_NOT_FOUND,
	EC_CHANNEL_NOT_FOUND,
	EC_CONNECTION_NOT_FOUND,
	EC_UNKNOWN_TRANSMISSION_MODE,
	EC_AUDIO_NOT_AVAILABLE,
	EC_INVALID_SAMPLE,
	EC_INVALID_PLUGIN_ID
};

enum PositionalDataErrorCode {
	/// Positional data has been initialized properly
	PDEC_OK = 0,
	/// Positional data is temporarily unavailable (e.g. because the corresponding process isn't running) but might be
	/// at another point in time.
	PDEC_ERROR_TEMP,
	/// Positional data is permanently unavailable (e.g. because the respective memory offsets are outdated).
	PDEC_ERROR_PERM
};

enum KeyCode {
	KC_INVALID           = -1,

	// Non-printable characters first
	KC_NULL              = 0,
	KC_END               = 1,
	KC_LEFT              = 2,
	KC_RIGHT             = 4,
	KC_UP                = 5,
	KC_DOWN              = 6,
	KC_DELETE            = 7,
	KC_BACKSPACE         = 8,
	KC_TAB               = 9,
	KC_ENTER             = 10, // == '\n'
	KC_ESCAPE            = 27,
	KC_PAGE_UP           = 11,
	KC_PAGE_DOWN         = 12,
	KC_SHIFT             = 13,
	KC_CONTROL           = 14,
	KC_META              = 15,
	KC_ALT               = 16,
	KC_ALT_GR            = 17,
	KC_CAPSLOCK          = 18,
	KC_NUMLOCK           = 19,
	KC_SUPER             = 20, // == windows key
	KC_HOME              = 21, // == Pos1
	KC_PRINT             = 22,
	KC_SCROLLLOCK        = 23,

	// Printable characters are assigned to their ASCII code
	KC_SPACE             = ' ',
	KC_EXCLAMATION_MARK  = '!',
	KC_DOUBLE_QUOTE      = '"',
	KC_HASHTAG           = '#',
	KC_DOLLAR            = '$',
	KC_PERCENT           = '%',
	KC_AMPERSAND         = '&',
	KC_SINGLE_QUOTE      = '\'',
	KC_OPEN_PARENTHESIS  = '(',
	KC_CLOSE_PARENTHESIS = ')',
	KC_ASTERISK          = '*',
	KC_PLUS              = '+',
	KC_COMMA             = ',',
	KC_MINUS             = '-',
	KC_PERIOD            = '.',
	KC_SLASH             = '/',
	KC_0                 = '0',
	KC_1                 = '1',
	KC_2                 = '2',
	KC_3                 = '3',
	KC_4                 = '4',
	KC_5                 = '5',
	KC_6                 = '6',
	KC_7                 = '7',
	KC_8                 = '8',
	KC_9                 = '9',
	KC_COLON             = ':',
	KC_SEMICOLON         = ';',
	KC_LESS_THAN         = '<',
	KC_EQUALS            = '=',
	KC_GREATER_THAN      = '>',
	KC_QUESTION_MARK     = '?',
	KC_AT_SYMBOL         = '@',
	KC_A                 = 'A',
	KC_B                 = 'B',
	KC_C                 = 'C',
	KC_D                 = 'D',
	KC_E                 = 'E',
	KC_F                 = 'F',
	KC_G                 = 'G',
	KC_H                 = 'H',
	KC_I                 = 'I',
	KC_J                 = 'J',
	KC_K                 = 'K',
	KC_L                 = 'L',
	KC_M                 = 'M',
	KC_N                 = 'N',
	KC_O                 = 'O',
	KC_P                 = 'P',
	KC_Q                 = 'Q',
	KC_R                 = 'R',
	KC_S                 = 'S',
	KC_T                 = 'T',
	KC_U                 = 'U',
	KC_V                 = 'V',
	KC_W                 = 'W',
	KC_X                 = 'X',
	KC_Y                 = 'Y',
	KC_Z                 = 'Z',
	// leave out lowercase letters (for now)
	KC_OPEN_BRACKET      = '[',
	KC_BACKSLASH         = '\\',
	KC_CLOSE_BRACKET     = ']',
	KC_CIRCUMFLEX        = '^',
	KC_UNDERSCORE        = '_',
	KC_GRAVE_AKCENT      = '`',
	KC_OPEN_BRACE        = '{',
	KC_VERTICAL_BAR      = '|',
	KC_CLOSE_BRACE       = '}',
	KC_TILDE             = '~',

	// Some characters from the extended ASCII code
	KC_DEGREE_SIGN       = 176,



	// F-keys
	// Start at a value of 256 as extended ASCII codes range up to 256
	KC_F1                = 256,
	KC_F2                = 257,
	KC_F3                = 258,
	KC_F4                = 259,
	KC_F5                = 260,
	KC_F6                = 261,
	KC_F7                = 262,
	KC_F8                = 263,
	KC_F9                = 264,
	KC_F10               = 265,
	KC_F11               = 266,
	KC_F12               = 267,
	KC_F13               = 268,
	KC_F14               = 269,
	KC_F15               = 270,
	KC_F16               = 271,
	KC_F17               = 272,
	KC_F18               = 273,
	KC_F19               = 274,
};

struct Version {
	int32_t major;
	int32_t minor;
	int32_t patch;
#ifdef __cplusplus
	bool operator<(const Version& other) const {
		return this->major <= other.major && this->minor <= other.minor && this->patch < other.patch;
	}

	bool operator>(const Version& other) const {
		return this->major >= other.major && this->minor >= other.minor && this->patch > other.patch;
	}

	bool operator>=(const Version& other) const {
		return this->major >= other.major && this->minor >= other.minor && this->patch >= other.patch;
	}

	bool operator<=(const Version& other) const {
		return this->major <= other.major && this->minor <= other.minor && this->patch <= other.patch;
	}

	bool operator==(const Version& other) const {
		return this->major == other.major && this->minor == other.minor && this->patch == other.patch;
	}

	operator std::string() const {
		return std::string("v") + std::to_string(this->major) + std::to_string(this->minor) + std::to_string(this->patch);
	}

#ifdef QT_VERSION
	operator QString() const {
		return QString::fromLatin1("v%0.%1.%2").arg(this->major).arg(this->minor).arg(this->patch);
	}
#endif
#endif
};

/// @param errorCode The error code to get a message for
/// @returns The error message coresponding to the given error code. The message
/// 	is encoded as a C-string and are static meaning that it is safe to use the
/// 	returned pointer in your code.
inline const char* errorMessage(int16_t errorCode) {
	switch (errorCode) {
		case EC_GENERIC_ERROR:
			return "Generic error";
		case EC_OK:
			return "Ok - this is not an error";
		case EC_POINTER_NOT_FOUND:
			return "Can't find the passed pointer";
		case EC_NO_ACTIVE_CONNECTION:
			return "There is currently no active connection to a server";
		case EC_USER_NOT_FOUND:
			return "Can't find the requested user";
		case EC_CHANNEL_NOT_FOUND:
			return "Can't find the requested channel";
		case EC_CONNECTION_NOT_FOUND:
			return "Can't identify the requested connection";
		case EC_UNKNOWN_TRANSMISSION_MODE:
			return "Unknown transmission mode encountered";
		case EC_AUDIO_NOT_AVAILABLE:
			return "There is currently no audio output available";
		case EC_INVALID_SAMPLE:
			return "Attempted to use invalid sample (can't play it)";
		case EC_INVALID_PLUGIN_ID:
			return "Used an invalid plugin ID";
		default:
			return "Unknown error code";
	}
}


typedef enum TalkingState talking_state_t;
typedef enum TransmissionMode transmission_mode_t;
typedef struct Version version_t;
typedef int32_t mumble_connection_t;
typedef uint32_t mumble_userid_t;
typedef int32_t mumble_channelid_t;
typedef enum ErrorCode mumble_error_t;
typedef uint32_t plugin_id_t;
typedef KeyCode keycode_t;


// API version
const int32_t MUMBLE_PLUGIN_API_MAJOR = 1;
const int32_t MUMBLE_PLUGIN_API_MINOR = 0;
const int32_t MUMBLE_PLUGIN_API_PATCH = 0;
const version_t MUMBLE_PLUGIN_API_VERSION = { MUMBLE_PLUGIN_API_MAJOR, MUMBLE_PLUGIN_API_MINOR, MUMBLE_PLUGIN_API_PATCH };


struct MumbleAPI {
	// -------- Memory management --------
	
	/// Frees the given pointer.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param pointer The pointer to free
	/// @returns The error code. If everything went well, STATUS_OK will be returned.
	mumble_error_t (PLUGIN_CALLING_CONVENTION *freeMemory)(plugin_id_t callerID, void *pointer);


	
	// -------- Getter functions --------

	/// Gets the connection ID of the server the user is currently active on (the user's audio output is directed at).
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param[out] connection A pointer to the memory location the ID should be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then it is valid to access the
	/// 	value of the provided pointer
	mumble_error_t (PLUGIN_CALLING_CONVENTION *getActiveServerConnection)(plugin_id_t callerID, mumble_connection_t *connection);

	/// Fills in the information about the local user.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param[out] userID A pointer to the memory the user's ID shall be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer
	/// 	may be accessed
	mumble_error_t (PLUGIN_CALLING_CONVENTION *getLocalUserID)(plugin_id_t callerID, mumble_connection_t connection, mumble_userid_t *userID);

	/// Fills in the information about the given user's name.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param userID The user's ID whose name should be obtained
	/// @param[out] userName A pointer to where the pointer to the allocated string (C-encoded) should be written to. The
	/// 	allocated memory has to be freed by a call to freeMemory by the plugin eventually. The memory will only be
	/// 	allocated if this function returns STATUS_OK.
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer
	/// 	may be accessed
	mumble_error_t (PLUGIN_CALLING_CONVENTION *getUserName)(plugin_id_t callerID, mumble_connection_t connection,
			mumble_userid_t userID, char **userName);

	/// Fills in the information about the given channel's name.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param channelID The channel's ID whose name should be obtained
	/// @param[out] channelName A pointer to where the pointer to the allocated string (C-ecoded) should be written to. The
	/// 	allocated memory has to be freed by a call to freeMemory by the plugin eventually. The memory will only be
	/// 	allocated if this function returns STATUS_OK.
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer
	/// 	may be accessed
	mumble_error_t (PLUGIN_CALLING_CONVENTION *getChannelName)(plugin_id_t callerID, mumble_connection_t connection,
			mumble_channelid_t channelID, char **channelName);

	/// Gets an array of all users that are currently connected to the provided server. Passing a nullptr as any of the out-parameter
	/// will prevent that property to be set/allocated. If you are only interested in the user count you can thus pass nullptr as the
	/// users parameter and save time on allocating + freeing the channels-array while still getting the size out.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param[out] users A pointer to where the pointer of the allocated array shall be written. The
	/// 	allocated memory has to be freed by a call to freeMemory by the plugin eventually. The memory will only be
	/// 	allocated if this function returns STATUS_OK.
	/// @param[out] userCount A pointer to where the size of the allocated user-array shall be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer
	/// 	may be accessed
	mumble_error_t (PLUGIN_CALLING_CONVENTION *getAllUsers)(plugin_id_t callerID, mumble_connection_t connection, mumble_userid_t **users,
			size_t *userCount);

	/// Gets an array of all channels on the provided server. Passing a nullptr as any of the out-parameter will prevent
	/// that property to be set/allocated. If you are only interested in the channel count you can thus pass nullptr as the
	/// channels parameter and save time on allocating + freeing the channels-array while still getting the size out.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param[out] channels A pointer to where the pointer of the allocated array shall be written. The
	/// 	allocated memory has to be freed by a call to freeMemory by the plugin eventually. The memory will only be
	/// 	allocated if this function returns STATUS_OK.
	/// @param[out] channelCount A pointer to where the size of the allocated channel-array shall be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer
	/// 	may be accessed
	mumble_error_t (PLUGIN_CALLING_CONVENTION *getAllChannels)(plugin_id_t callerID, mumble_connection_t connection,
			mumble_channelid_t **channels, size_t *channelCount);

	/// Gets the ID of the channel the given user is currently connected to.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param userID The ID of the user to search for
	/// @param[out] A pointer to where the ID of the channel shall be written
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer
	/// 	may be accessed
	mumble_error_t (PLUGIN_CALLING_CONVENTION *getChannelOfUser)(plugin_id_t callerID, mumble_connection_t connection, mumble_userid_t userID,
			mumble_channelid_t *channel);

	/// Gets an array of all users in the specified channel.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param channelID The ID of the channel whose users shall be retrieved
	/// @param[out] userList A pointer to where the pointer of the allocated array shall be written. The allocated memory has
	/// 	to be freed by a call to freeMemory by the plugin eventually. The memory will only be allocated if this function
	/// 	returns STATUS_OK.
	/// @param[out] userCount A pointer to where the size of the allocated user-array shall be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer
	/// 	may be accessed
	mumble_error_t (PLUGIN_CALLING_CONVENTION *getUsersInChannel)(plugin_id_t callerID, mumble_connection_t connection,
			mumble_channelid_t channelID, mumble_userid_t **userList, size_t *userCount);

	/// Gets the current transmission mode of the local user.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param[out] transmissionMode A pointer to where the transmission mode shall be written.
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer
	/// 	may be accessed
	mumble_error_t (PLUGIN_CALLING_CONVENTION *getLocalUserTransmissionMode)(plugin_id_t callerID, transmission_mode_t *transmissionMode);



	// -------- Request functions --------
	
	/// Requests Mumble to set the local user's transmission mode to the specified one. If you only need to temporarily set
	/// the transmission mode to continous, use requestMicrophoneActivationOverwrite instead as this saves you the work of
	/// restoring the previous state afterwards.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param transmissionMode The requested transmission mode
	/// @returns The error code. If everything went well, STATUS_OK will be returned.
	mumble_error_t (PLUGIN_CALLING_CONVENTION *requestLocalUserTransmissionMode)(plugin_id_t callerID, transmission_mode_t transmissionMode);

	/// Requests Mumble to move the given user into the given channel
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param userID The ID of the user that shall be moved
	/// @param channelID The ID of the channel to move the user to
	/// @param password The password of the target channel (UTF-8 encoded as a C-string). Pass NULL if the target channel does not require a
	/// 	password for entering
	/// @returns The error code. If everything went well, STATUS_OK will be returned.
	mumble_error_t (PLUGIN_CALLING_CONVENTION *requestUserMove)(plugin_id_t callerID, mumble_connection_t connection, mumble_userid_t userID,
			mumble_channelid_t channelID, const char *password);

	/// Requests Mumble to overwrite the microphone activation so that the microphone is always on (same as if the user had chosen
	/// the continous transmission mode). If a plugin requests this overwrite, it is responsible for deactivating the overwrite again
	/// once it is no longer required
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param activate Whether to activate the overwrite (false deactivates an existing overwrite)
	/// @returns The error code. If everything went well, STATUS_OK will be returned.
	mumble_error_t (PLUGIN_CALLING_CONVENTION *requestMicrophoneActivationOvewrite)(plugin_id_t callerID, bool activate);



	// -------- Find functions --------
	
	/// Fills in the information about a user with the specified name, if such a user exists. The search is case-sensitive.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param userName The respective user's name
	/// @param[out] userID A pointer to the memory the user's ID shall be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer may
	/// 	be accessed.
	mumble_error_t (PLUGIN_CALLING_CONVENTION *findUserByName)(plugin_id_t callerID, mumble_connection_t connection, const char *userName,
			mumble_userid_t *userID);

	/// Fills in the information about a channel with the specified name, if such a channel exists. The search is case-sensitive.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to use as a context
	/// @param channelName The respective channel's name
	/// @param[out] channelID A pointer to the memory the channel's ID shall be written to
	/// @returns The error code. If everything went well, STATUS_OK will be returned. Only then the passed pointer may
	/// 	be accessed.
	mumble_error_t (PLUGIN_CALLING_CONVENTION *findChannelByName)(plugin_id_t callerID, mumble_connection_t connection,
			const char *channelName, mumble_channelid_t *channelID);



	// -------- Miscellaneous --------
	
	/// Sends the provided data to the provided client(s). This kind of data can only be received by another plugin active
	/// on that client. The sent data can be seen by any active plugin on the receiving client. Therefore the sent data
	/// must not contain sensitive information or anything else that shouldn't be known by others.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param connection The ID of the server-connection to send the data through (the server the given users are on)
	/// @param users An array of user IDs to send the data to
	/// @param userCount The size of the provided user-array
	/// @param data The data that shall be sent as a String
	/// @param dataLength The length of the data-string
	/// @param dataID The ID of the sent data. This has to be used by the receiving plugin(s) to figure out what to do with
	/// 	the data
	/// @returns The error code. If everything went well, STATUS_OK will be returned.
	mumble_error_t (PLUGIN_CALLING_CONVENTION *sendData)(plugin_id_t callerID, mumble_connection_t connection, mumble_userid_t *users,
			size_t userCount, const char *data, size_t dataLength, const char *dataID);

	/// Logs the given message (typically to Mumble's console). All passed strings have to be UTF-8 encoded.
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param message The message to log
	/// @returns The error code. If everything went well, STATUS_OK will be returned.
	mumble_error_t (PLUGIN_CALLING_CONVENTION *log)(plugin_id_t callerID, const char *message);

	/// Plays the provided sample. It uses libsndfile as a backend so the respective file format needs to be supported by it
	/// in order for this to work out (see http://www.mega-nerd.com/libsndfile/).
	///
	/// @param callerID The ID of the plugin calling this function
	/// @param samplePath The path to the sample that shall be played (UTF-8 encoded)
	/// @returns The error code. If everything went well, STATUS_OK will be returned.
	mumble_error_t (PLUGIN_CALLING_CONVENTION *playSample)(plugin_id_t callerID, const char *samplePath);
};


#endif
