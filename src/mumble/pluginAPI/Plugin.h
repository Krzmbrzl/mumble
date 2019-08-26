/**
 * This is a first draft for a plugin interface
 */

#ifndef MUMBLE_PLUGIN_H_
#define MUMBLE_PLUGIN_H_

#include "PluginComponents.h"

#ifdef WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

	// functions for init and de-init
	PLUGIN_EXPORT Status init();
	PLUGIN_EXPORT void shutdown();

	// The intention is to call this function as a first step whenever loading a plugin - It's good habit
	// to tell your partner who you are first, right?
	PLUGIN_EXPORT void setMumbleInfo(Version MumbleVersion, Version mumbleAPIVersion, Version minimalExpectedAPIVersion);

	// functions for general plugin info
	PLUGIN_EXPORT const char* name();
	PLUGIN_EXPORT const char* getDisplayVersion();
	PLUGIN_EXPORT Version getVersion();
	PLUGIN_EXPORT Version getRequiredApiVersion();
	PLUGIN_EXPORT const char* getAuthor();
	PLUGIN_EXPORT const char* getDescription();
	PLUGIN_EXPORT void setFunctionPointers(const struct MumbleFunctions functions);



	// Tell the plugin the ID by which it is referenced by Mumble
	PLUGIN_EXPORT void registerPluginID(const char* id);


	// Parameters to functions below are yet to be determined

	// -------- Callback functions -----------

	/// Called whenever the user changes his/her channel.
	PLUGIN_EXPORT void onChannelChanged();
	/// Called when connecting to a server.
	PLUGIN_EXPORT void onServerConnect();
	/// Called when disconnecting from a server.
	PLUGIN_EXPORT void onServerDisconnect();
	/// Called when a new client connects to the server the user is currently connected to.
	PLUGIN_EXPORT void onNewClientConnectedToServer();
	/// Called when a client disconnects from the server the user is currently connected to.
	PLUGIN_EXPORT void onClientDisconnectedFromServer();
	/// Called when a client joins the channel the user is currently in.
	PLUGIN_EXPORT void onClientJoinedChannel();
	/// Called when a client leaves the channel the user is currently in.
	PLUGIN_EXPORT void onClientLeftChannel();
	/// Called when the user changes his/her username
	PLUGIN_EXPORT void onUsernameChanged();
	/// Called whenever there is audio input
	PLUGIN_EXPORT void onAudioInput(short* inputPCM, bool isSpeech);
	/// Called whenever there is audio output
	PLUGIN_EXPORT void onAudioOutput_short(short* outputPCM, int sampleCount, int channelCount);
	/// Called whenever there is audio output
	PLUGIN_EXPORT void onAudioOutput_float(float* outputPCM, int sampleCount, int channelCount);


#ifdef __cplusplus
}
#endif 


#endif
