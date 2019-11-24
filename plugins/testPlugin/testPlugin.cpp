// Copyright 2005-2019 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

// Include the definitions of the plugin functions
// Not that this will also include ../PluginComponents.h
#include "../MumblePlugin.h"

#include <iostream>
#include <cstring>

// These are just some utility functions facilitating writing logs and the like
// The actual implementation of the plugin is further down
std::ostream& pLog() {
	std::cout << "TestPlugin: ";
	return std::cout;
}

template<typename T>
void pluginLog(T log) {
	pLog() << log << std::endl;
}

std::ostream& operator<<(std::ostream& stream, const Version_t version) {
	stream << "v" << version.major << "." << version.minor << "." << version.patch;
	return stream;
}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////// PLUGIN IMPLEMENTATION ///////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

MumbleAPI mumAPI;
MumbleConnection_t activeConnection;

//////////////////////////////////////////////////////////////
//////////////////// OBLIGATORY FUNCTIONS ////////////////////
//////////////////////////////////////////////////////////////
// All of the following function must be implemented in order for Mumble to load the plugin

MumbleError_t init() {
	pluginLog("Initialized plugin");

	// STATUS_OK is a macro set to the appropriate status flag (ErrorCode)
	// If you need to return any other status have a look at the ErrorCode enum
	// inside PluginComponents.h and use one of its values
	return STATUS_OK;
}

void shutdown() {
	pluginLog("Shutdown plugin");

	mumAPI.log("testPlugin", "Shutdown");
}

const char* getName() {
	// The pointer returned by this functions has to remain valid forever and it must be able to return
	// one even if the plugin hasn't loaded (yet). Thus it may not require any variables that are only set
	// once the plugin is initialized
	// For most cases returning a hard-coded String-literal should be what you aim for
	return "TestPlugin";
}

Version_t getAPIVersion() {
	// MUMBLE_PLUGIN_API_VERSION will always contain the API version of the used header file (the one used to build
	// this plugin against). Thus you should always return that here in order to no have to worry about it.
	return MUMBLE_PLUGIN_API_VERSION;
}

void registerAPIFunctions(MumbleAPI api) {
	// In this function the plugin is presented with a struct of function pointers that can be used
	// to interact with Mumble. Thus you should store it somewhere safe for later usage.
	mumAPI = api;

	pluginLog("Registered Mumble's API functions");

	mumAPI.log("testPlugin", "Received API functions");
}


//////////////////////////////////////////////////////////////
///////////////////// OPTIONAL FUNCTIONS /////////////////////
//////////////////////////////////////////////////////////////
// The implementation of below functions is optional. If you don't need them, don't include them in your
// plugin

void setMumbleInfo(Version_t mumbleVersion, Version_t mumbleAPIVersion, Version_t minimalExpectedAPIVersion) {
	// this function will always be the first one to be called. Even before init()
	// In here you can get info about the Mumble version this plugin is about to run in.
	pLog() << "Mumble version: " << mumbleVersion << "; Mumble API-Version: " << mumbleAPIVersion << "; Minimal expected API-Version: "
		<< minimalExpectedAPIVersion << std::endl;
}

Version_t getVersion() {
	// Mumble uses semantic versioning (see https://semver.org/)
	// { major, minor, patch }
	return { 1, 0, 0 };
}

const char* getAuthor() {
	// For the returned pointer the same rules as for getName() apply
	// In short: in the vast majority of cases you'll want to return a hard-coded String-literal
	return "MumbleDevelopers";
}

const char* getDescription() {
	// For the returned pointer the same rules as for getName() apply
	// In short: in the vast majority of cases you'll want to return a hard-coded String-literal
	return "This plugin is merely a reference implementation without any real functionality. It shouldn't be included in the release build of Mumble";
}

void registerPluginID(uint32_t id) {
	// This ID serves as an identifier for this plugin as far as Mumble is concerned
	// It might be a good idea to store it somewhere for later use
	pLog() << "Registered ID: " << id << std::endl;
}

uint32_t getFeatures() {
	// Tells Mumble whether this plugin delivers some known common functionality. See the PluginFeature enum in
	// PluginComponents.h for what is available.
	// If you want your plugin to deliver positional data, you'll want to return FEATURE_POSITIONAL
	return FEATURE_NONE;
}

uint32_t deactivateFeatures(uint32_t features) {
	pLog() << "Asked to deactivate feature set " << features << std::endl;

	// All features that can't be deactivated should be returned
	return features;
}

uint8_t initPositionalData(const char **programNames, const uint64_t *programPIDs, size_t programCount) {
	std::ostream& stream = pLog() << "Got " << programCount << " programs to init positional data.";

	if (programCount > 0) {
		stream << " The first name is " << programNames[0] << " and has PID " << programPIDs[0];
	}

	stream << std::endl;

	// As this plugin doesn't provide PD, we return PDEC_ERROR_PERM to indicate that even in the future we won't do so
	// If your plugin is indeed delivering positional data but is only temporarily unaible to do so, return PDEC_ERROR_TEMP
	// and if you deliver PD and succeeded initializing return PDEC_OK.
	return PDEC_ERROR_PERM;
}

#define SET_TO_ZERO(name) name[0] = 0.0f; name[1] = 0.0f; name[2] = 0.0f
bool fetchPositionalData(float *avatarPos, float *avatarDir, float *avatarAxis, float *cameraPos, float *cameraDir,
			float *cameraAxis, const char **context, const char **identity) {
	pluginLog("Has been asked to deliver positional data");

	// If unable to provide positional data, this function should return false and reset all given values to 0/empty Strings
	SET_TO_ZERO(avatarPos);
	SET_TO_ZERO(avatarDir);
	SET_TO_ZERO(avatarAxis);
	SET_TO_ZERO(cameraPos);
	SET_TO_ZERO(cameraDir);
	SET_TO_ZERO(cameraAxis);
	*context = "";
	*identity = "";

	// This function returns whether it can continue to deliver positional data
	return false;
}

void shutdownPositionalData() {
	pluginLog("Shutting down positional data");
}

void onServerConnected(MumbleConnection_t connection) {
	activeConnection = connection;

	pLog() << "Established server-connection with ID " << connection << std::endl;
}

void onServerDisconnected(MumbleConnection_t connection) {
	activeConnection = -1;

	pLog() << "Disconnected from server-connection with ID " << connection << std::endl;
}

void onServerSynchronized(MumbleConnection_t connection) {
	// The client has finished synchronizing with the server. Thus we can now obtain a list of all users on this server
	pLog() << "Server has finished synchronizing (ServerConnection: " << connection << ")" << std::endl ;

	size_t userCount;
	MumbleUserID_t *userIDs;

	if (mumAPI.getAllUsers(activeConnection, &userIDs, &userCount) != STATUS_OK) {
		pluginLog("[ERROR]: Can't obtain user list");
		return;
	}

	pLog() << "There are " << userCount << " users on this server. Their names are:" << std::endl;

	for(size_t i=0; i<userCount; i++) {
		char *userName;
		mumAPI.getUserName(connection, userIDs[i], &userName);
		
		pLog() << "\t" << userName << std::endl;

		mumAPI.freeMemory(userName);
	}

	mumAPI.freeMemory(userIDs);

	MumbleUserID_t localUser;
	if (mumAPI.getLocalUserID(activeConnection, &localUser) != STATUS_OK) {
		pluginLog("Failed to retrieve local user ID");
		return;
	}

	if (mumAPI.sendData(activeConnection, &localUser, 1, "Just a test", 12, "testMsg") == STATUS_OK) {
		pluginLog("Successfully sent plugin message");
	} else {
		pluginLog("Failed at sending message");
	}
}

void onChannelEntered(MumbleConnection_t connection, MumbleUserID_t userID, MumbleChannelID_t previousChannelID, MumbleChannelID_t newChannelID) {
	std::ostream& stream = pLog() << "User with ID " << userID << " entered channel with ID " << newChannelID << ".";

	// negative ID means that there was no previous channel (e.g. because the user just connected)
	if (previousChannelID >= 0) {
		stream << " He came from channel with ID " << previousChannelID << ".";
	}

	stream << " (ServerConnection: " << connection << ")" << std::endl;
}

void onChannelExited(MumbleConnection_t connection, MumbleUserID_t userID, MumbleChannelID_t channelID) {
	pLog() << "User with ID " << userID << " has left channel with ID " << channelID << ". (ServerConnection: " << connection << ")" << std::endl;
}

void onUserTalkingStateChanged(MumbleConnection_t connection, MumbleUserID_t userID, TalkingState_t talkingState) {
	std::ostream& stream = pLog() << "User with ID " << userID << " changed his talking state to ";

	// The possible values are contained in the TalkingState enum inside PluginComponent.h
	switch(talkingState) {
		case INVALID:
			stream << "Invalid";
			break;
		case PASSIVE:
			stream << "Passive";
			break;
		case TALKING:
			stream << "Talking";
			break;
		case WHISPERING:
			stream << "Whispering";
			break;
		case SHOUTING:
			stream << "Shouting";
			break;
		default:
			stream << "Unknown (" << talkingState << ")";
	}

	stream << ". (ServerConnection: " << connection << ")" << std::endl;
}

bool onAudioInput(short *inputPCM, uint32_t sampleCount, uint16_t channelCount, bool isSpeech) {
	pLog() << "Audio input with " << channelCount << " channels and " << sampleCount << " samples per channel encountered. IsSpeech: "
		<< isSpeech << std::endl;

	// mark inputPCM as unused
	(void) inputPCM;

	// This function returns whether it has modified the audio stream
	return false;
}

bool onAudioSourceFetched(float *outputPCM, uint32_t sampleCount, uint16_t channelCount, bool isSpeech, MumbleUserID_t userID) {
	std::ostream& stream = pLog() << "Audio output source with " << channelCount << " channels and " << sampleCount << " samples per channel fetched.";

	if (isSpeech) {
		stream << " The output is speech from user with ID " << userID << ".";
	}

	stream << std::endl;

	// Mark ouputPCM as unused
	(void) outputPCM;

	// This function returns whether it has modified the audio stream
	return false;
}

bool onAudioOutputAboutToPlay(float *outputPCM, uint32_t sampleCount, uint16_t channelCount) {
	pLog() << "The resulting audio output has " << channelCount << " channels with " << sampleCount << " samples per channel" << std::endl;

	// mark outputPCM as unused
	(void) outputPCM;

	// This function returns whether it has modified the audio stream
	return false;
}

bool onReceiveData(MumbleConnection_t connection, MumbleUserID_t sender, const char *data, size_t dataLength, const char *dataID) {
	pLog() << "Received data with ID \"" << dataID << "\" from user with ID " << sender << ". Its length is " << dataLength
		<< ". (ServerConnection:" << connection << ")" << std::endl;

	if (std::strcmp(dataID, "testMsg") == 0) {
		pLog() << "The received data: " << data << std::endl;
	}

	// This function returns whether it has processed the data (preventing further plugins from seeing it)
	return false;
}

void onUserAdded(MumbleConnection_t connection, MumbleUserID_t userID) {
	pLog() << "Added user with ID " << userID << " (ServerConnection: " << connection << ")" << std::endl;
}

void onUserRemoved(MumbleConnection_t connection, MumbleUserID_t userID) {
	pLog() << "Removed user with ID " << userID << " (ServerConnection: " << connection << ")" << std::endl;
}

void onChannelAdded(MumbleConnection_t connection, MumbleChannelID_t channelID) {
	pLog() << "Added channel with ID " << channelID << " (ServerConnection: " << connection << ")" << std::endl;
}

void onChannelRemoved(MumbleConnection_t connection, MumbleChannelID_t channelID) {
	pLog() << "Removed channel with ID " << channelID << " (ServerConnection: " << connection << ")" << std::endl;
}

void onChannelRenamed(MumbleConnection_t connection, MumbleChannelID_t channelID) {
	pLog() << "Renamed channel with ID " << channelID << " (ServerConnection: " << connection << ")" << std::endl;
}
