/// @file pluginAPI/Plugin.h

/*
 * This is a first draft for a plugin interface
 */

#ifndef MUMBLE_PLUGIN_H_
#define MUMBLE_PLUGIN_H_

#include "PluginComponents.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

	// functions for init and de-init
	
	/// Gets called right after loading the plugin in order to let the plugin initialize.
	///
	/// @returns The status of the initialization. If everything went fine, return STATUS_OK
	PLUGIN_EXPORT error_t init();
	
	/// Gets called when unloading the plugin in order to allow it to clean up after itself.
	PLUGIN_EXPORT void shutdown();

	/// Tells the plugin some basic information about the Mumble client loading it.
	/// This function will be the first one that is being called on this plugin - even before it is decided whether to load
	/// the plugin at all.
	///
	/// @param mumbleVersion The Version of the Mumble client
	/// @param mumbleAPIVersion The Version of the plugin-API the Mumble client runs with
	/// @param minimalExpectedAPIVersion The minimal Version the Mumble clients expects this plugin to meet in order to load it
	PLUGIN_EXPORT void setMumbleInfo(Version_t mumbleVersion, Version_t mumbleAPIVersion, Version_t minimalExpectedAPIVersion);

	// functions for general plugin info
	
	/// Gets the name of the plugin. The plugin has to guarantee that the returned pointer will still be valid. The string will be copied
	/// for further usage though.
	///
	/// @returns A pointer to the plugin name (encoded as a C-String)
	PLUGIN_EXPORT const char* getName();

	/// Gets the plugin's version in a display-ready representation.
	///
	/// @param[out] versionBuffer The buffer into which the version string shall be copied by this function
	/// @param bufferSize The size of the buffer - The length of the version (including terminating zero-byte) must not exceed this length
	PLUGIN_EXPORT void getDisplayVersion(char *versionBuffer, size_t bufferSize);

	/// Gets the Version of this plugin
	///
	/// @returns The plugin's version
	PLUGIN_EXPORT Version_t getVersion();

	/// Gets the Version of the plugin-API this plugin intends to use.
	/// Mumble will decide whether this plugin is loadable or not based on the return value of this function.
	///
	/// @return The respective API Version
	PLUGIN_EXPORT Version_t getApiVersion();

	/// Gets the name of the plugin author(s). The plugin has to guarantee that the returned pointer will still be valid. The string will
	/// be copied for further usage though.
	///
	/// @returns A pointer to the author(s) name(s) (encoded as a C-String)
	PLUGIN_EXPORT const char* getAuthor();

	/// Gets the description of the plugin. The plugin has to guarantee that the returned pointer will still be valid. The string will
	/// be copied for further usage though.
	///
	/// @returns A pointer to the description (encoded as a C-String)
	PLUGIN_EXPORT const char* getDescription();

	/// Provides the MumbleAPI struct to the plugin. This struct contains function pointers that can be used
	/// to interact with the Mumble client. It is up to the plugin to store this struct somewhere if it wants to make use
	/// of it at some point.
	///
	/// @param api The MumbleAPI struct
	PLUGIN_EXPORT void registerAPIFunctions(const struct MumbleAPI *api);

	/// Registers the ID of this plugin. This is the ID Mumble will reference this plugin with and by which this plugin
	/// can identify itself when communicating with Mumble.
	///
	/// @param id The ID for this plugin
	PLUGIN_EXPORT void registerPluginID(uint32_t id);


	// Parameters to functions below are yet to be determined

	// -------- Callback functions -----------

	/// Called when connecting to a server.
	PLUGIN_EXPORT void onServerConnected();

	/// Called when disconnecting from a server.
	PLUGIN_EXPORT void onServerDisconnected();

	/// Called whenever any user on the server enters a channel
	/// This function will also be called when freshly connecting to a server as each user on that
	/// server needs to be "added" to the respective channel as far as the local client is concerned.
	///
	/// @param userID The ID of the user this event has been triggered for
	/// @param previousChannelID The ID of the chanel the user is coming from. Negative IDs indicate that there is no previous channel (e.g. the user
	/// 	freshly connected to the server) or the channel isn't available because of any other reason.
	/// @param newChannelID The ID of the channel the user has entered. If the ID is negative, the new channel could not be retrieved. This means
	/// 	that the ID is invalid.
	PLUGIN_EXPORT void onChannelEntered(uint32_t userID, int32_t previousChannelID, int32_t newChannelID);

	/// Called whenever a user leaves a channel.
	/// This function will also be called when the user disconnects from the server.
	///
	/// @param userID The ID of the user that left the channel
	/// @param channelID The ID of the channel the user left. If the ID is negative, the channel could not be retrieved. This means that the ID is
	/// 	invalid.
	PLUGIN_EXPORT void onChannelExited(uint32_t userID, int32_t channelID);

	/// Called when any user changes his/her talking state.
	///
	/// @param userID The ID of the user whose talking state has been changed
	/// @param talkingState The new TalkingState the user has switched to.
	PLUGIN_EXPORT void onUserTalkingStateChanged(uint32_t userID, TalkingState_t talkingState);

	/// Called whenever there is audio input
	PLUGIN_EXPORT void onAudioInput(short *inputPCM, bool isSpeech);

	/// Called whenever there is audio output
	PLUGIN_EXPORT void onAudioOutput_short(short *outputPCM, uint32_t sampleCount, uint32_t channelCount);

	/// Called whenever there is audio output
	PLUGIN_EXPORT void onAudioOutput_float(float *outputPCM, uint32_t sampleCount, uint32_t channelCount);


#ifdef __cplusplus
}
#endif 


#endif
