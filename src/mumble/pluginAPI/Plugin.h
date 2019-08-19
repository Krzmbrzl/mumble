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

	// functions for general plugin info
	PLUGIN_EXPORT const char* name();
	PLUGIN_EXPORT const char* getVersion();
	PLUGIN_EXPORT int getNumericVersion(); // This could facilitate programmatic version comparisons
	PLUGIN_EXPORT int getRequiredApiVersion();
	PLUGIN_EXPORT const char* getAuthor();
	PLUGIN_EXPORT const char* getDescription();
	PLUGIN_EXPORT void setFunctionPointers(const struct MumbleFunctions functions);



	// Tell the plugin the ID by which it is referenced by Mumble
	PLUGIN_EXPORT void registerPluginID(const char* id);


	// Parameters to functions below are yet to be determined

	// Callback functions
	PLUGIN_EXPORT void onChannelChanged();
	PLUGIN_EXPORT void onConnectedServerChanged();
	PLUGIN_EXPORT void onServerConnect();
	PLUGIN_EXPORT void onServerDisconnect();
	PLUGIN_EXPORT void onNewClientConnectedToServer();
	PLUGIN_EXPORT void onClientDisconnectedFromServer();
	PLUGIN_EXPORT void onClientJoinedChannel();
	PLUGIN_EXPORT void onClientLeftChannel();
	PLUGIN_EXPORT void onUsernameChanged();


#ifdef __cplusplus
}
#endif 


#endif
