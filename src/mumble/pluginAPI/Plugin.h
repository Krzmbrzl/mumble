/// @file pluginAPI/Plugin.h

/*
 * This is a first draft for a plugin interface
 */

#ifndef MUMBLE_PLUGIN_H_
#define MUMBLE_PLUGIN_H_

#include "PluginComponents.h"
#include <stdint.h>
#include <stddef.h>

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
	PLUGIN_EXPORT Status init();
	
	/// Gets called when unloading the plugin in order to allow it to clean up after itself.
	PLUGIN_EXPORT void shutdown();

	/// Tells the plugin some basic information about the Mumble client loading it.
	/// This function will be the first one that is being called on this plugin - even before it is decided whether to load
	/// the plugin at all.
	///
	/// @param mumbleVersion The Version of the Mumble client
	/// @param mumbleAPIVersion The Version of the plugin-API the Mumble client runs with
	/// @param minimalExpectedAPIVersion The minimal Version the Mumble clients expects this plugin to meet in order to load it
	PLUGIN_EXPORT void setMumbleInfo(Version mumbleVersion, Version mumbleAPIVersion, Version minimalExpectedAPIVersion);

	// functions for general plugin info
	
	/// Gets the name of the plugin.
	///
	/// @param[out] nameBuffer The buffer into which the name of the plugin shall be copied by this function
	/// @param bufferSize The size of the buffer - The length of the name of the plugin (including the terminating zero-byte) must not exceed
	/// 	this length
	PLUGIN_EXPORT void getName(char *nameBuffer, size_t bufferSize);

	/// Gets the plugin's version in a display-ready representation.
	///
	/// @param[out] versionBuffer The buffer into which the version string shall be copied by this function
	/// @param bufferSize The size of the buffer - The length of the version (including terminating zero-byte) must not exceed this length
	PLUGIN_EXPORT void getDisplayVersion(char *versionBuffer, size_t bufferSize);

	/// Gets the Version of this plugin
	///
	/// @returns The plugin's version
	PLUGIN_EXPORT struct Version getVersion();

	/// Gets the Version of the plugin-API this plugin intends to use.
	/// Mumble will decide whether this plugin is loadable or not based on the return value of this function.
	///
	/// @return The respective API Version
	PLUGIN_EXPORT struct Version getApiVersion();

	/// Gets the name of the plugin author(s)
	///
	/// @param[out] authorBuffer The buffer into which the author name shall be copied by this function
	/// @param bufferSize The size of the buffer - The length of the author name (including the terminating zero-byte) must not exceed
	/// 	this length.
	PLUGIN_EXPORT void getAuthor(char *authorBuffer, size_t bufferSize);

	/// Gets the description of the plugin
	///
	/// @param[out] descriptionBuffer The buffer into which the description shall be copied by this function
	/// @param bufferSize The siue of the buffer - The length of the description (including the terminating zero-byte) must not exceed
	/// 	this length.
	PLUGIN_EXPORT void getDescription(char *descriptionBuffer, size_t bufferSize);

	/// Provides the MumbleFunctions struct to the plugin. This struct contains function pointers that can be used
	/// to interact with the Mumble client. It is up to the plugin to store this struct somewhere if it wants to make use
	/// of it at some point.
	///
	/// @param functions The MumbleFunctions struct
	PLUGIN_EXPORT void setFunctionPointers(const struct MumbleFunctions *functions);



	/// Sets the ID of this plugin. This is the ID Mumble will reference this plugin with and by which this plugin
	/// can identify itself when communicating with Mumble.
	///
	/// @param id The ID for this plugin
	PLUGIN_EXPORT void setPluginID(int32_t id);


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
	/// 	freshly connected to the server)
	/// @param newChannelID The ID of the channel the user has entered
	PLUGIN_EXPORT void onChannelEntered(uint32_t userID, int32_t previousChannelID, int32_t newChannelID);

	/// Called whenever a user leaves a channel.
	/// This function will also be called when the user disconnects from the server.
	///
	/// @param userID The ID of the user that left the channel
	/// @param channelID The ID of the channel the user left
	PLUGIN_EXPORT void onChannelExited(uint32_t userID, int32_t channelID);

	/// Called when the user changes his/her username
	PLUGIN_EXPORT void onUsernameChanged();

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
