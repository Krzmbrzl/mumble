/**
 * This is a first draft for a plugin interface
 */

#ifndef MUMBLE_PLUGIN_H_
#define MUMBLE_PLUGIN_H_

#ifdef WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__ ((visibility("default")))
#endif

namespace MumblePlugin {

	// Define an enum for error codes
	enum Status {
		OK=0,
		ERROR
	};

#ifdef __cplusplus
	extern "C" {
#endif

		// functions for init and de-init
		PLUGIN_EXPORT Status init();
		PLUGIN_EXPORT Status shutdown();

		// functions for general plugin info
		PLUGIN_EXPORT const char* getName();
		PLUGIN_EXPORT const char* getVersion();
		PLUGIN_EXPORT const char* getAuthor();

#ifdef __cplusplus
	}
#endif 

} // MumblePlugin

#endif
