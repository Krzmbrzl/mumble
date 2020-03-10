// Copyright 2005-2019 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "LegacyPlugin.h"
#include <cstdlib>
#include <wchar.h>
#include <map>
#include <string.h>
#include <codecvt>
#include <locale>

#include <QRegularExpression>


/// A regular expression used to extract the version from the legacy plugin's description
static const QRegularExpression versionRegEx(QString::fromLatin1("(?:v)?(?:ersion)?[ \\t]*(\\d+)\\.(\\d+)(?:\\.(\\d+))?"), QRegularExpression::CaseInsensitiveOption);


LegacyPlugin::LegacyPlugin(QString path, bool isBuiltIn, QObject *p) : Plugin(path, isBuiltIn, p), name(), description(), version(VERSION_UNKNOWN), mumPlug(0),
	mumPlug2(0), mumPlugQt(0) {
}

LegacyPlugin::~LegacyPlugin() {
}

bool LegacyPlugin::doInitialize() {
	if (Plugin::doInitialize()) {
		// initialization seems to have succeeded so far
		// This means that mumPlug is initialized
		
		this->name = QString::fromStdWString(this->mumPlug->shortname);
		// Although the MumblePlugin struct has a member called "description", the actual description seems to
		// always only be returned by the longdesc function (The description member is actually just the name with some version
		// info)
		this->description = QString::fromStdWString(this->mumPlug->longdesc());
		// The version field in the MumblePlugin2 struct is the positional-audio-plugin-API version and not the version
		// of the plugin itself. This information is not provided for legacy plugins.
		// Most of them however provide information about the version of the game they support. Thus we will try to parse the
		// description and extract this version using it for the plugin's version as well.
		// Some plugins have the version in the actual description field of the old API (see above comment why these aren't the same)
		// sp we will use a combination of both to search for the version. If multiple version(-like) strings are found, the last one
		// will be used.
		QString matchContent = this->description + QChar::Null + QString::fromStdWString(this->mumPlug->description);
		QRegularExpressionMatchIterator matchIt = versionRegEx.globalMatch(matchContent);

		// Only consider the last match
		QRegularExpressionMatch match;
		while (matchIt.hasNext()) {
			match = matchIt.next();
		}

		if (match.hasMatch()) {
			// Store version
			this->version = { match.captured(1).toInt(), match.captured(2).toInt(), match.captured(3).toInt() };	
		}

		return true;
	} else {
		// initialization has failed
		// pass on info about failed init
		return false;
	}
}

void LegacyPlugin::resolveFunctionPointers() {
	// We don't set any functions inside the apiFnc struct variable in order for the default
	// implementations in the Plugin class to mimic empty default implementations for all functions
	// not explicitly overwritten by this class
	QWriteLocker lock(&this->pluginLock);

	if (this->isValid()) {
		// The corresponding library was loaded -> try to locate all API functions of the legacy plugin's spec
		// (for positional audio) and set defaults for the other ones in order to maintain compatibility with
		// the new plugin system
		
		mumblePluginFunc pluginFunc = reinterpret_cast<mumblePluginFunc>(this->lib.resolve("getMumblePlugin"));	
		mumblePlugin2Func plugin2Func = reinterpret_cast<mumblePlugin2Func>(this->lib.resolve("getMumblePlugin2"));	
		mumblePluginQtFunc pluginQtFunc = reinterpret_cast<mumblePluginQtFunc>(this->lib.resolve("getMumblePluginQt"));	

		if (pluginFunc) {
			this->mumPlug = pluginFunc();
		}
		if (plugin2Func) {
			this->mumPlug2 = plugin2Func();
		}
		if (pluginQtFunc) {
			this->mumPlugQt = pluginQtFunc();
		}

		// A legacy plugin is valid as long as there is a function to get the MumblePlugin struct from it
		// and the plugin has been compiled by the same compiler as this client (determined by the plugin's
		// "magic") and it isn't retracted
		bool suitableMagic = this->mumPlug && this->mumPlug->magic == MUMBLE_PLUGIN_MAGIC;
		bool retracted = this->mumPlug && this->mumPlug->shortname == L"Retracted";
		this->pluginIsValid = pluginFunc && suitableMagic && !retracted;

#ifdef MUMBLE_PLUGIN_DEBUG
		if (!this->pluginIsValid) {
			if (!pluginFunc) {
				qDebug("Plugin \"%s\" is missing the getMumblePlugin() function", qPrintable(this->pluginPath));
			} else if (!suitableMagic) {
				qDebug("Plugin \"%s\" was compiled with a different compiler (magic differs)", qPrintable(this->pluginPath));
			} else {
				qDebug("Plugin \"%s\" is retracted", qPrintable(this->pluginPath));
			}
		}
#endif
	}
}

QString LegacyPlugin::getName() const {
	PluginReadLocker lock(&this->pluginLock);

	if (!this->name.isEmpty()) {
		return this->name;
	} else {
		return QString::fromUtf8("Unknown Legacy Plugin");
	}	
}

QString LegacyPlugin::getDescription() const {
	PluginReadLocker lock(&this->pluginLock);

	if (!this->description.isEmpty()) {
		return this->description;
	} else {
		return QString::fromUtf8("No description provided by the legacy plugin");
	}
}

bool LegacyPlugin::showAboutDialog(QWidget *parent) const {
	PluginReadLocker lock(&this->pluginLock);

	if (this->mumPlugQt && this->mumPlugQt->about) {
		this->mumPlugQt->about(parent);

		return true;
	}
	if (this->mumPlug->about) {
		// the original implementation in Mumble would pass nullptr to the about-function in the mumPlug struct
		// so we'll mimic that behaviour for compatibility
		this->mumPlug->about(nullptr);

		return true;
	}

	return false;
}

bool LegacyPlugin::showConfigDialog(QWidget *parent) const {
	PluginReadLocker lock(&this->pluginLock);

	if (this->mumPlugQt && this->mumPlugQt->config) {
		this->mumPlugQt->config(parent);

		return true;
	}
	if (this->mumPlug->config) {
		// the original implementation in Mumble would pass nullptr to the about-function in the mumPlug struct
		// so we'll mimic that behaviour for compatibility
		this->mumPlug->config(nullptr);

		return true;
	}

	return false;
}

uint8_t LegacyPlugin::initPositionalData(const char **programNames, const uint64_t *programPIDs, size_t programCount) {
	PluginReadLocker rLock(&this->pluginLock);

	int retCode;

	if (this->mumPlug2) {
		// Create and populate a multimap holding the names and PIDs to pass to the tryLock-function
		std::multimap<std::wstring, unsigned long long int> pidMap;

		for (size_t i=0; i<programCount; i++) {
			std::string currentName = programNames[i];
			std::wstring currentNameWstr = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(currentName);

			pidMap.insert(std::pair<std::wstring, unsigned long long int>(currentNameWstr, programPIDs[i]));
		}

		retCode = this->mumPlug2->trylock(pidMap);
	} else {
		// The default MumblePlugin doesn't take the name and PID arguments
		retCode = this->mumPlug->trylock();
	}

	// ensure that only expected return codes are being returned from this function
	// the legacy plugins return 1 on successfull locking and 0 on failure
	if (retCode) {
		rLock.unlock();
		QWriteLocker wLock(&this->pluginLock);

		this->positionalDataIsActive = true;

		return PDEC_OK;
	} else {
		// legacy plugins don't have the concept of indicating a permanent error
		// so we'll return a temporary error for them
		return PDEC_ERROR_TEMP;
	}
}

bool LegacyPlugin::fetchPositionalData(Position3D& avatarPos, Vector3D& avatarDir, Vector3D& avatarAxis, Position3D& cameraPos, Vector3D& cameraDir,
		Vector3D& cameraAxis, QString& context, QString& identity) {
	PluginReadLocker lock(&this->pluginLock);

	std::wstring identityWstr;
	std::string contextStr;

	int retCode = this->mumPlug->fetch(static_cast<float*>(avatarPos), static_cast<float*>(avatarDir), static_cast<float*>(avatarAxis),
			static_cast<float*>(cameraPos), static_cast<float*>(cameraDir), static_cast<float*>(cameraAxis), contextStr, identityWstr);

	context = QString::fromStdString(contextStr);
	identity = QString::fromStdWString(identityWstr);

	// The fetch-function should return if it is "still locked on" meaning that it can continue providing
	// positional audio
	return retCode == 1;
}

void LegacyPlugin::shutdownPositionalData() {
	QWriteLocker lock(&this->pluginLock);

	this->positionalDataIsActive = false;

	this->mumPlug->unlock();
}

uint32_t LegacyPlugin::getFeatures() const {
	return FEATURE_POSITIONAL;
}

version_t LegacyPlugin::getVersion() const {
	return this->version;
}

bool LegacyPlugin::providesAboutDialog() const {
	PluginReadLocker lock(&this->pluginLock);

	return this->mumPlug->about || (this->mumPlugQt && this->mumPlugQt->about);
}

bool LegacyPlugin::providesConfigDialog() const {
	PluginReadLocker lock(&this->pluginLock);

	return this->mumPlug->config || (this->mumPlugQt && this->mumPlugQt->config);
}

version_t LegacyPlugin::getAPIVersion() const {
	// Legacy plugins are always on most recent API as they don't use it in any case -> no need to perform
	// backwards compatibility stuff
	return MUMBLE_PLUGIN_API_VERSION;
}