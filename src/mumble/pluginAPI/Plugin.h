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
	PLUGIN_EXPORT const char* version();
	PLUGIN_EXPORT int apiVersion();
	PLUGIN_EXPORT const char* author();
	PLUGIN_EXPORT const char* description();
	PLUGIN_EXPORT void setFunctionPointers(const struct MumbleFunctions functions);



	PLUGIN_EXPORT void registerPluginID(const char* id);


#ifdef __cplusplus
}
#endif 


#endif
