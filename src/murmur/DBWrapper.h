// Copyright 2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_SERVER_DBWRAPPER_H_
#define MUMBLE_SERVER_DBWRAPPER_H_

#include "murmur/database/DBLogEntry.h"
#include "murmur/database/DBUserData.h"
#include "murmur/database/ServerDatabase.h"
#include "murmur/database/UserProperty.h"

#include "database/ConnectionParameter.h"

#include "Ban.h"
#include "ServerUserInfo.h"
#include "User.h"

#include <boost/optional.hpp>

#include <exception>
#include <string>
#include <vector>

class Server;
class ServerUser;
class Channel;
class Meta;
class ChannelListenerManager;

class QByteArray;

class DBWrapper {
public:
	DBWrapper(const ::mumble::db::ConnectionParameter &connectionParams);

	// Server management
	std::vector< unsigned int > getAllServers();
	std::vector< unsigned int > getBootServers();
	unsigned int addServer();
	void removeServer(unsigned int serverID);
	bool serverExists(unsigned int serverID);
	void setServerBootProperty(unsigned int serverID, bool boot);

	void setSuperUserPassword(unsigned int serverID, const std::string &password);
	void disableSuperUser(unsigned int serverID);

	void clearAllPerServerSLLConfigurations();
	void clearAllServerLogs();

	std::vector< Ban > getBans(unsigned int serverID);
	void saveBans(unsigned int serverID, const std::vector< Ban > &bans);

	void initializeChannels(Server &server);
	void initializeChannelDetails(Server &server);
	void initializeChannelLinks(Server &server);

	unsigned int getNextAvailableChannelID(unsigned int serverID);
	void createChannel(unsigned int serverID, const Channel &channel);
	void deleteChannel(unsigned int serverID, unsigned int channelID);
	void updateChannelData(unsigned int serverID, const Channel &channel);

	void addChannelLink(unsigned int serverID, const Channel &first, const Channel &second);
	void removeChannelLink(unsigned int serverID, const Channel &first, const Channel &second);


	void getConfigurationTo(unsigned int serverID, const std::string &configKey, QString &outVar);
	void getConfigurationTo(unsigned int serverID, const std::string &configKey, std::string &outVar);
	void getConfigurationTo(unsigned int serverID, const std::string &configKey, QByteArray &outVar);
	void getConfigurationTo(unsigned int serverID, const std::string &configKey, unsigned short &outVar);
	void getConfigurationTo(unsigned int serverID, const std::string &configKey, bool &outVar);
	void getConfigurationTo(unsigned int serverID, const std::string &configKey, int &outVar);
	void getConfigurationTo(unsigned int serverID, const std::string &configKey, unsigned int &outVar);
	void getConfigurationTo(unsigned int serverID, const std::string &configKey, boost::optional< bool > &outVar);

	std::vector< std::pair< std::string, std::string > > getAllConfigurations(unsigned int serverID);

	void setConfiguration(unsigned int serverID, const std::string &configKey, const std::string &value);
	void clearConfiguration(unsigned int serverID, const std::string &configKey);


	void logMessage(unsigned int serverID, const std::string &msg);
	std::vector< mumble::server::db::DBLogEntry > getLogs(unsigned int serverID, unsigned int startOffset = 0,
														  int amount = -1);
	std::size_t getLogSize(unsigned int serverID);

	/**
	 * Sets the last-disconnected status of the given user to the current time
	 */
	void updateLastDisconnect(unsigned int serverID, unsigned int userID);

	void addChannelListenerIfNotExists(unsigned int serverID, const ServerUserInfo &userInfo, const Channel &channel);
	void disableChannelListenerIfExists(unsigned int serverID, const ServerUserInfo &userInfo, const Channel &channel);
	void deleteChannelListener(unsigned int serverID, const ServerUserInfo &userInfo, const Channel &channel);
	void loadChannelListenersOf(unsigned int serverID, const ServerUserInfo &userInfo, ChannelListenerManager &manager);
	void storeChannelListenerVolume(unsigned int serverID, const ServerUserInfo &userInfo, const Channel &channel,
									float volumeFactor);

	/**
	 * Performs the registration of the given user in the database
	 *
	 * @returns The user ID assigned to the newly registered user
	 */
	unsigned int registerUser(unsigned int serverID, const ServerUserInfo &userInfo);
	void unregisterUser(unsigned int serverID, unsigned int userID);
	int registeredUserNameToID(unsigned int serverID, const std::string &name);
	bool registeredUserExists(unsigned int serverID, unsigned int userID);
	QMap< int, QString > getRegisteredUserDetails(unsigned int serverID, unsigned int userID);
	void addAllRegisteredUserInfoTo(std::vector< UserInfo > &userInfo, unsigned int serverID,
									const std::string &nameFilter);

	void setLastChannel(unsigned int serverID, const ServerUserInfo &userInfo);
	int getLastChannelID(unsigned int serverID, const ServerUserInfo &userInfo);


	QByteArray getUserTexture(unsigned int serverID, const ServerUserInfo &userInfo);
	void storeUserTexture(unsigned int serverID, const ServerUserInfo &userInfo);

	std::string getUserProperty(unsigned int serverID, const ServerUserInfo &userInfo,
								::mumble::server::db::UserProperty property);
	void storeUserProperty(unsigned int serverID, const ServerUserInfo &userInfo,
						   ::mumble::server::db::UserProperty prop, const std::string &value);
	void setUserProperties(unsigned int serverID, unsigned int userID,
						   const std::vector< std::pair< unsigned int, std::string > > &properties);
	std::vector< std::pair< unsigned int, std::string > > getUserProperties(unsigned int serverID, unsigned int userID);

	std::string getUserName(unsigned int serverID, unsigned int userID);

	unsigned int getNextAvailableUserID(unsigned int serverID);

	void setUserData(unsigned int serverID, unsigned int userID, const ::mumble::server::db::DBUserData &data);

protected:
	::mumble::server::db::ServerDatabase m_serverDB;
};

#endif // MUMBLE_SERVER_DBWRAPPER_H_