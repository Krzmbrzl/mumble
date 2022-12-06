// Copyright 2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "DBWrapper.h"
#include "ACL.h"
#include "Channel.h"
#include "ChannelListenerManager.h"
#include "Group.h"
#include "Meta.h"
#include "MumbleConstants.h"
#include "PasswordGenerator.h"
#include "Server.h"
#include "ServerUser.h"
#include "VolumeAdjustment.h"

#include "database/Exception.h"
#include "database/FormatException.h"
#include "database/NoDataException.h"
#include "database/TransactionHolder.h"

#include "murmur/database/ACLTable.h"
#include "murmur/database/BanTable.h"
#include "murmur/database/ChannelLinkTable.h"
#include "murmur/database/ChannelListenerTable.h"
#include "murmur/database/ChannelProperty.h"
#include "murmur/database/ChannelPropertyTable.h"
#include "murmur/database/ChannelTable.h"
#include "murmur/database/ChronoUtils.h"
#include "murmur/database/ConfigTable.h"
#include "murmur/database/DBAcl.h"
#include "murmur/database/DBBan.h"
#include "murmur/database/DBChannel.h"
#include "murmur/database/DBChannelLink.h"
#include "murmur/database/DBChannelListener.h"
#include "murmur/database/DBGroup.h"
#include "murmur/database/DBGroupMember.h"
#include "murmur/database/DBLogEntry.h"
#include "murmur/database/DBUser.h"
#include "murmur/database/GroupMemberTable.h"
#include "murmur/database/GroupTable.h"
#include "murmur/database/LogTable.h"
#include "murmur/database/ServerTable.h"
#include "murmur/database/UserPropertyTable.h"
#include "murmur/database/UserTable.h"

#include <iostream>
#include <limits>
#include <stdexcept>

#include <QDateTime>
#include <QString>
#include <utility>

namespace mdb  = ::mumble::db;
namespace msdb = ::mumble::server::db;

DBWrapper::DBWrapper(const ::mdb::ConnectionParameter &connectionParams)
	: m_serverDB(connectionParams.applicability()) {
	// Immediately initialize the database connection
	m_serverDB.init(connectionParams);
}

void printExceptionMessage(std::ostream &stream, const std::exception &e, int indent = 0) {
	for (int i = 0; i < indent; ++i) {
		stream << " ";
	}

	stream << e.what();

	try {
		std::rethrow_if_nested(e);
	} catch (const std::exception &nestedExc) {
		stream << "\n";
		indent += 2;

		printExceptionMessage(stream, nestedExc, indent);
	}
}

#define WRAPPER_BEGIN try {
// Our error handling consists in properly printing the encountered error and then throwing
// a standard std::exception that should be caught in our QCoreApplication's notify function,
// which we have overridden to exit all event processing and thereby shutting down all servers.
#define WRAPPER_END                                                       \
	}                                                                     \
	catch (const ::mdb::Exception &e) {                                   \
		std::cerr << "[ERROR]: Encountered database error:" << std::endl; \
		printExceptionMessage(std::cerr, e, 1);                           \
		std::cerr << std::endl;                                           \
                                                                          \
		throw std::runtime_error("Database error");                       \
	}

std::vector< unsigned int > DBWrapper::getAllServers() {
	WRAPPER_BEGIN

	return m_serverDB.getServerTable().getAllServerIDs();

	WRAPPER_END
}

std::vector< unsigned int > DBWrapper::getBootServers() {
	WRAPPER_BEGIN

	std::vector< unsigned int > bootIDs;

	for (unsigned int id : m_serverDB.getServerTable().getAllServerIDs()) {
		bool boot = false;
		getConfigurationTo(id, "boot", boot);

		if (boot) {
			bootIDs.push_back(id);
		}
	}

	return bootIDs;

	WRAPPER_END
}

unsigned int DBWrapper::addServer() {
	WRAPPER_BEGIN

	unsigned int serverID = m_serverDB.getServerTable().getFreeServerID();

	m_serverDB.getServerTable().addServer(serverID);

	// Ensure that the root channel exists
	::msdb::DBChannel rootChannel;
	rootChannel.serverID  = serverID;
	rootChannel.channelID = Mumble::ROOT_CHANNEL_ID;
	rootChannel.name      = "Root";
	m_serverDB.getChannelTable().addChannel(rootChannel);

	// Ensure that a SuperUser entry exists
	::msdb::DBUser superUser(serverID, Mumble::SUPERUSER_ID);
	m_serverDB.getUserTable().addUser(superUser, "SuperUser");

	// Generate a new, default password for the SuperUser
	constexpr const int pwSize = 32;
	std::string pw             = PasswordGenerator::generatePassword(pwSize).toStdString();
	setSuperUserPassword(serverID, pw);

	// Write the default password into the DB, in case it needs to be fetched at a later point
	logMessage(serverID, "Initialized 'SuperUser' password on server " + std::to_string(serverID) + " to '" + pw + "'");

	return serverID;

	WRAPPER_END
}

void DBWrapper::removeServer(unsigned int serverID) {
	WRAPPER_BEGIN

	m_serverDB.getServerTable().removeServer(serverID);

	WRAPPER_END
}

bool DBWrapper::serverExists(unsigned int serverID) {
	WRAPPER_BEGIN

	return m_serverDB.getServerTable().serverExists(serverID);

	WRAPPER_END
}

void DBWrapper::setServerBootProperty(unsigned int serverID, bool boot) {
	WRAPPER_BEGIN

	m_serverDB.getConfigTable().setConfig(serverID, "boot", std::to_string(boot));

	WRAPPER_END
}

void DBWrapper::setSuperUserPassword(unsigned int serverID, const std::string &password) {
	WRAPPER_BEGIN

	const ::msdb::DBUser superUser(serverID, Mumble::SUPERUSER_ID);

	m_serverDB.getUserTable().setPassword(superUser, password);

	WRAPPER_END
}

void DBWrapper::disableSuperUser(unsigned int serverID) {
	WRAPPER_BEGIN

	const ::msdb::DBUser superUser(serverID, Mumble::SUPERUSER_ID);

	m_serverDB.getUserTable().clearPassword(superUser);

	WRAPPER_END
}

void DBWrapper::clearAllPerServerSLLConfigurations() {
	WRAPPER_BEGIN

	for (unsigned int serverID : getAllServers()) {
		for (const std::string &currentKey :
			 std::vector< std::string >{ "key", "certificate", "passphrase", "sslDHParams" }) {
			clearConfiguration(serverID, currentKey);
		}
	}

	WRAPPER_END
}

void DBWrapper::clearAllServerLogs() {
	WRAPPER_BEGIN

	for (unsigned int serverID : getAllServers()) {
		m_serverDB.getLogTable().clearLog(serverID);
	}

	WRAPPER_END
}

std::vector< Ban > DBWrapper::getBans(unsigned int serverID) {
	WRAPPER_BEGIN

	std::vector< Ban > bans;

	for (const ::msdb::DBBan &currentBan : m_serverDB.getBanTable().getAllBans(serverID)) {
		assert(currentBan.serverID == serverID);

		Ban ban;

		ban.iDuration = currentBan.duration.count();
		ban.iMask     = currentBan.prefixLength;
		ban.qdtStart  = QDateTime::fromSecsSinceEpoch(::msdb::toEpochSeconds(currentBan.startDate));
		ban.haAddress = HostAddress(currentBan.baseAddress);
		if (currentBan.reason) {
			ban.qsReason = QString::fromStdString(currentBan.reason.get());
		}
		if (currentBan.bannedUserCertHash) {
			ban.qsHash = QString::fromStdString(currentBan.bannedUserCertHash.get());
		}
		if (currentBan.bannedUserName) {
			ban.qsUsername = QString::fromStdString(currentBan.bannedUserName.get());
		}

		bans.push_back(std::move(ban));
	}

	return bans;

	WRAPPER_END
}

void DBWrapper::saveBans(unsigned int serverID, const std::vector< Ban > &bans) {
	WRAPPER_BEGIN

	std::vector<::msdb::DBBan > dbBans;
	dbBans.reserve(bans.size());

	for (const Ban &currentBan : bans) {
		::msdb::DBBan dbBan;

		dbBan.serverID     = serverID;
		dbBan.duration     = std::chrono::seconds(currentBan.iDuration);
		dbBan.prefixLength = currentBan.iMask;
		dbBan.startDate =
			std::chrono::system_clock::time_point(std::chrono::seconds(currentBan.qdtStart.toSecsSinceEpoch()));
		dbBan.baseAddress = currentBan.haAddress.getByteRepresentation();
		if (!currentBan.qsHash.isEmpty()) {
			dbBan.bannedUserCertHash = currentBan.qsHash.toStdString();
		}
		if (!currentBan.qsUsername.isEmpty()) {
			dbBan.bannedUserName = currentBan.qsUsername.toStdString();
		}
		if (!currentBan.qsReason.isEmpty()) {
			dbBan.reason = currentBan.qsReason.toStdString();
		}

		dbBans.push_back(std::move(dbBan));
	}

	m_serverDB.getBanTable().setBans(serverID, dbBans);

	WRAPPER_END
}

void readChildren(::msdb::ServerDatabase &db, Channel *parent, Server &server) {
	for (unsigned int currentChildID : db.getChannelTable().getChildrenOf(server.iServerNum, parent->iId)) {
		::msdb::DBChannel channelInfo = db.getChannelTable().getChannelData(server.iServerNum, currentChildID);

		Channel *currentChild     = new Channel(currentChildID, QString::fromStdString(channelInfo.name), parent);
		currentChild->bInheritACL = channelInfo.inheritACL;

		server.qhChannels.insert(currentChildID, currentChild);

		// Recurse
		readChildren(db, currentChild, server);
	}
}

void DBWrapper::initializeChannels(Server &server) {
	WRAPPER_BEGIN

	::msdb::DBChannel root = m_serverDB.getChannelTable().getChannelData(server.iServerNum, Mumble::ROOT_CHANNEL_ID);

	Channel *rootChannel     = new Channel(Mumble::ROOT_CHANNEL_ID, QString::fromStdString(root.name), &server);
	rootChannel->bInheritACL = root.inheritACL;

	server.qhChannels.insert(rootChannel->iId, rootChannel);

	readChildren(m_serverDB, rootChannel, server);

	initializeChannelDetails(server);

	WRAPPER_END
}

void DBWrapper::initializeChannelDetails(Server &server) {
	WRAPPER_BEGIN

	for (Channel *currentChannel : server.qhChannels) {
		assert(currentChannel);

		// Read and set channel properties
		std::string description = m_serverDB.getChannelPropertyTable().getProperty< std::string, false >(
			server.iServerNum, currentChannel->iId, ::msdb::ChannelProperty::Description);
		if (!description.empty()) {
			Server::hashAssign(currentChannel->qsDesc, currentChannel->qbaDescHash,
							   QString::fromStdString(description));
		}

		currentChannel->iPosition = m_serverDB.getChannelPropertyTable().getProperty< int, false >(
			server.iServerNum, currentChannel->iId, ::msdb::ChannelProperty::Position);

		currentChannel->uiMaxUsers = m_serverDB.getChannelPropertyTable().getProperty< unsigned int, false >(
			server.iServerNum, currentChannel->iId, ::msdb::ChannelProperty::MaxUsers);


		// Read and initialize the groups defined for the current channel
		for (const ::msdb::DBGroup &currentGroup :
			 m_serverDB.getGroupTable().getAllGroups(server.iServerNum, currentChannel->iId)) {
			Group *group        = new Group(currentChannel, QString::fromStdString(currentGroup.name));
			group->bInherit     = currentGroup.inherit;
			group->bInheritable = currentGroup.is_inheritable;

			for (const ::msdb::DBGroupMember &currrentMember :
				 m_serverDB.getGroupMemberTable().getEntries(server.iServerNum, currentGroup.groupID)) {
				if (currrentMember.addToGroup) {
					group->qsAdd << currrentMember.userID;
				} else {
					group->qsRemove << currrentMember.userID;
				}
			}
		}

		// Read and set access control lists
		for (const ::msdb::DBAcl &currentAcl :
			 m_serverDB.getACLTable().getAllACLs(server.iServerNum, currentChannel->iId)) {
			ChanACL *acl = new ChanACL(currentChannel);
			acl->iUserId = currentAcl.affectedUserID ? static_cast< int >(currentAcl.affectedUserID.get()) : -1;
			if (currentAcl.affectedGroupID) {
				acl->qsGroup = QString::fromStdString(
					m_serverDB.getGroupTable().getGroup(server.iServerNum, currentAcl.affectedGroupID.get()).name);
			}
			acl->bApplyHere = currentAcl.applyInCurrentChannel;
			acl->bApplySubs = currentAcl.applyInSubChannels;
			acl->pAllow     = static_cast< ChanACL::Permissions >(currentAcl.grantedPrivilegeFlags);
			acl->pDeny      = static_cast< ChanACL::Permissions >(currentAcl.revokedPrivilegeFlags);
		}
	}

	WRAPPER_END
}

void DBWrapper::initializeChannelLinks(Server &server) {
	WRAPPER_BEGIN

	for (const ::msdb::DBChannelLink &currentLink : m_serverDB.getChannelLinkTable().getAllLinks(server.iServerNum)) {
		Channel *first  = server.qhChannels.value(currentLink.firstChannelID);
		Channel *second = server.qhChannels.value(currentLink.secondChannelID);

		if (first && second) {
			// Linking A to B will automatically link B to A as well
			first->link(second);
		}
	}

	WRAPPER_END
}

unsigned int DBWrapper::getNextAvailableChannelID(unsigned int serverID) {
	WRAPPER_BEGIN

	return m_serverDB.getChannelTable().getFreeChannelID(serverID);

	WRAPPER_END
}

::msdb::DBChannel channelToDB(unsigned int serverID, const Channel &channel) {
	::msdb::DBChannel dbChannel;
	dbChannel.serverID   = serverID;
	dbChannel.channelID  = channel.iId;
	dbChannel.name       = channel.qsName.toStdString();
	dbChannel.parentID   = channel.cParent ? channel.cParent->iId : channel.iId;
	dbChannel.inheritACL = channel.bInheritACL;

	return dbChannel;
}

::msdb::DBGroup groupToDB(unsigned int serverID, unsigned int groupID, const Group &group) {
	::msdb::DBGroup dbGroup;
	dbGroup.serverID       = serverID;
	dbGroup.groupID        = groupID;
	dbGroup.name           = group.qsName.toStdString();
	dbGroup.inherit        = group.bInherit;
	dbGroup.is_inheritable = group.bInheritable;

	return dbGroup;
}

::msdb::DBAcl aclToDB(unsigned int serverID, unsigned int priority, boost::optional< unsigned int > groupID,
					  const ChanACL &acl) {
	::msdb::DBAcl dbAcl;
	assert(acl.c);

	dbAcl.serverID              = serverID;
	dbAcl.channelID             = acl.c->iId;
	dbAcl.priority              = priority;
	dbAcl.applyInCurrentChannel = acl.bApplyHere;
	dbAcl.applyInSubChannels    = acl.bApplySubs;
	dbAcl.affectedGroupID       = std::move(groupID);
	if (acl.iUserId >= 0) {
		dbAcl.affectedUserID = acl.iUserId;
	}
	dbAcl.grantedPrivilegeFlags = static_cast< unsigned int >(acl.pAllow);
	dbAcl.revokedPrivilegeFlags = static_cast< unsigned int >(acl.pDeny);

	return dbAcl;
}

void DBWrapper::createChannel(unsigned int serverID, const Channel &channel) {
	WRAPPER_BEGIN

	// Add the given channel to the DB
	::msdb::DBChannel dbChannel = channelToDB(serverID, channel);

	m_serverDB.getChannelTable().addChannel(dbChannel);

	// Add channel properties to DB
	if (!channel.qsDesc.isEmpty()) {
		m_serverDB.getChannelPropertyTable().setProperty(serverID, channel.iId, ::msdb::ChannelProperty::Description,
														 channel.qsDesc.toStdString());
	} else {
		m_serverDB.getChannelPropertyTable().clearProperty(serverID, channel.iId, ::msdb::ChannelProperty::Description);
	}

	m_serverDB.getChannelPropertyTable().setProperty(serverID, channel.iId, ::msdb::ChannelProperty::Position,
													 std::to_string(channel.iPosition));

	m_serverDB.getChannelPropertyTable().setProperty(serverID, channel.iId, ::msdb::ChannelProperty::MaxUsers,
													 std::to_string(channel.uiMaxUsers));

	WRAPPER_END
}

void DBWrapper::deleteChannel(unsigned int serverID, unsigned int channelID) {
	WRAPPER_BEGIN

	m_serverDB.getChannelTable().removeChannel(serverID, channelID);

	WRAPPER_END
}

void DBWrapper::updateChannelData(unsigned int serverID, const Channel &channel) {
	WRAPPER_BEGIN

	if (channel.bTemporary) {
		// Temporary channels by definition are not stored in the DB
		return;
	}

	// Wrap all actions in a single transaction
	::mdb::TransactionHolder transaction = m_serverDB.ensureTransaction();

	::msdb::DBChannel dbChannel = channelToDB(serverID, channel);

	// Update channel object itself
	m_serverDB.getChannelTable().updateChannel(dbChannel);

	// Update channel properties
	if (!channel.qsDesc.isEmpty()) {
		m_serverDB.getChannelPropertyTable().setProperty(serverID, channel.iId, ::msdb::ChannelProperty::Description,
														 channel.qsDesc.toStdString());
	} else {
		m_serverDB.getChannelPropertyTable().clearProperty(serverID, channel.iId, ::msdb::ChannelProperty::Description);
	}

	m_serverDB.getChannelPropertyTable().setProperty(serverID, channel.iId, ::msdb::ChannelProperty::Position,
													 std::to_string(channel.iPosition));

	m_serverDB.getChannelPropertyTable().setProperty(serverID, channel.iId, ::msdb::ChannelProperty::MaxUsers,
													 std::to_string(channel.uiMaxUsers));

	// First, clear old groups and ACLs
	// (Clearing the groups automatically clear all entries referencing that group - in particular any members of that
	// group)
	m_serverDB.getGroupTable().clearGroups(serverID, channel.iId);
	m_serverDB.getACLTable().clearACLs(serverID, channel.iId);

	// Add current groups with their member information
	for (const Group *currentGroup : channel.qhGroups) {
		assert(currentGroup);

		unsigned int groupID = m_serverDB.getGroupTable().getFreeGroupID(serverID);

		::msdb::DBGroup group = groupToDB(serverID, groupID, *currentGroup);

		m_serverDB.getGroupTable().addGroup(group);

		for (int addedGroupMemberID : currentGroup->qsAdd) {
			assert(addedGroupMemberID >= 0);
			m_serverDB.getGroupMemberTable().addEntry(serverID, groupID, addedGroupMemberID, true);
		}
		for (int removeddGroupMemberID : currentGroup->qsRemove) {
			assert(removeddGroupMemberID >= 0);
			m_serverDB.getGroupMemberTable().addEntry(serverID, groupID, removeddGroupMemberID, false);
		}
	}


	// Why start at 5? Because the legacy code did so! ¯\_(ツ)_/¯
	unsigned int priority = 5;

	for (const ChanACL *currentACL : channel.qlACL) {
		assert(currentACL);

		boost::optional< unsigned int > associatedGroupID;
		if (!currentACL->qsGroup.isEmpty()) {
			associatedGroupID = m_serverDB.getGroupTable().findGroupID(serverID, currentACL->qsGroup.toStdString());

			if (!associatedGroupID) {
				throw ::mdb::NoDataException("Required ID of non-existing group \"" + currentACL->qsGroup.toStdString()
											 + "\"");
			}
		}

		::msdb::DBAcl acl = aclToDB(serverID, priority++, std::move(associatedGroupID), *currentACL);

		m_serverDB.getACLTable().addACL(acl);
	}

	transaction.commit();

	WRAPPER_END
}

void DBWrapper::addChannelLink(unsigned int serverID, const Channel &first, const Channel &second) {
	WRAPPER_BEGIN

	::msdb::DBChannelLink link(serverID, first.iId, second.iId);

	m_serverDB.getChannelLinkTable().addLink(link);

	WRAPPER_END
}

void DBWrapper::removeChannelLink(unsigned int serverID, const Channel &first, const Channel &second) {
	WRAPPER_BEGIN

	::msdb::DBChannelLink link(serverID, first.iId, second.iId);

	m_serverDB.getChannelLinkTable().removeLink(link);

	WRAPPER_END
}

void DBWrapper::getConfigurationTo(unsigned int serverID, const std::string &configKey, QString &outVar) {
	WRAPPER_BEGIN

	std::string property = m_serverDB.getConfigTable().getConfig(serverID, configKey);

	if (!property.empty()) {
		outVar = QString::fromStdString(property);
	}

	WRAPPER_END
}

void DBWrapper::getConfigurationTo(unsigned int serverID, const std::string &configKey, std::string &outVar) {
	WRAPPER_BEGIN

	std::string property = m_serverDB.getConfigTable().getConfig(serverID, configKey);

	if (!property.empty()) {
		outVar = std::move(property);
	}

	WRAPPER_END
}

void DBWrapper::getConfigurationTo(unsigned int serverID, const std::string &configKey, QByteArray &outVar) {
	WRAPPER_BEGIN

	std::string property = m_serverDB.getConfigTable().getConfig(serverID, configKey);

	if (!property.empty()) {
		outVar = QByteArray::fromStdString(property);
	}

	WRAPPER_END
}

void DBWrapper::getConfigurationTo(unsigned int serverID, const std::string &configKey, unsigned short &outVar) {
	WRAPPER_BEGIN

	std::string property = m_serverDB.getConfigTable().getConfig(serverID, configKey);

	if (!property.empty()) {
		try {
			outVar = std::stoul(property);
		} catch (const std::invalid_argument &) {
			std::throw_with_nested(
				::mdb::FormatException("Fetched property for key \"" + configKey + "\" can't be parsed as a number"));
		}
	}

	WRAPPER_END
}

bool stringToBool(const std::string &str) {
	return boost::iequals(str, "true") || str == "1";
}

void DBWrapper::getConfigurationTo(unsigned int serverID, const std::string &configKey, bool &outVar) {
	WRAPPER_BEGIN

	std::string property = m_serverDB.getConfigTable().getConfig(serverID, configKey);

	if (!property.empty()) {
		outVar = stringToBool(property);
	}

	WRAPPER_END
}

void DBWrapper::getConfigurationTo(unsigned int serverID, const std::string &configKey, int &outVar) {
	WRAPPER_BEGIN

	std::string property = m_serverDB.getConfigTable().getConfig(serverID, configKey);

	if (!property.empty()) {
		try {
			outVar = std::stoi(property);
		} catch (const std::invalid_argument &) {
			std::throw_with_nested(
				::mdb::FormatException("Fetched property for key \"" + configKey + "\" can't be parsed as a number"));
		}
	}

	WRAPPER_END
}

void DBWrapper::getConfigurationTo(unsigned int serverID, const std::string &configKey, unsigned int &outVar) {
	WRAPPER_BEGIN

	std::string property = m_serverDB.getConfigTable().getConfig(serverID, configKey);

	if (!property.empty()) {
		try {
			outVar = std::stoul(property);
		} catch (const std::invalid_argument &) {
			std::throw_with_nested(
				::mdb::FormatException("Fetched property for key \"" + configKey + "\" can't be parsed as a number"));
		}
	}

	WRAPPER_END
}

void DBWrapper::getConfigurationTo(unsigned int serverID, const std::string &configKey,
								   boost::optional< bool > &outVar) {
	WRAPPER_BEGIN

	std::string property = m_serverDB.getConfigTable().getConfig(serverID, configKey);

	outVar = property.empty() ? boost::none : boost::optional< bool >(stringToBool(property));

	WRAPPER_END
}

std::vector< std::pair< std::string, std::string > > DBWrapper::getAllConfigurations(unsigned int serverID) {
	WRAPPER_BEGIN

	std::vector< std::pair< std::string, std::string > > configs;

	for (std::pair< std::string, std::string > currentConfig : m_serverDB.getConfigTable().getAllConfigs(serverID)) {
		configs.push_back(std::move(currentConfig));
	}

	return configs;

	WRAPPER_END
}

void DBWrapper::setConfiguration(unsigned int serverID, const std::string &configKey, const std::string &value) {
	WRAPPER_BEGIN

	m_serverDB.getConfigTable().setConfig(serverID, configKey, value);

	WRAPPER_END
}

void DBWrapper::clearConfiguration(unsigned int serverID, const std::string &configKey) {
	WRAPPER_BEGIN

	m_serverDB.getConfigTable().clearConfig(serverID, configKey);

	WRAPPER_END
}

void DBWrapper::logMessage(unsigned int serverID, const std::string &msg) {
	WRAPPER_BEGIN

	::msdb::DBLogEntry entry(msg);

	m_serverDB.getLogTable().logMessage(serverID, entry);

	WRAPPER_END
}

std::vector<::msdb::DBLogEntry > DBWrapper::getLogs(unsigned int serverID, unsigned int startOffset, int amount) {
	WRAPPER_BEGIN

	return m_serverDB.getLogTable().getLogs(serverID, amount >= 0 ? amount : std::numeric_limits< int >::max(),
											startOffset);

	WRAPPER_END
}

std::size_t DBWrapper::getLogSize(unsigned int serverID) {
	WRAPPER_BEGIN

	return m_serverDB.getLogTable().getLogSize(serverID);

	WRAPPER_END
}

void DBWrapper::updateLastDisconnect(unsigned int serverID, unsigned int userID) {
	WRAPPER_BEGIN

	::msdb::DBUser user(serverID, userID);

	m_serverDB.getUserTable().setLastDisconnect(user);

	WRAPPER_END
}

void DBWrapper::addChannelListenerIfNotExists(unsigned int serverID, const ServerUserInfo &userInfo,
											  const Channel &channel) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);

	::msdb::DBChannelListener listener(serverID, channel.iId, userInfo.iId);

	if (!m_serverDB.getChannelListenerTable().listenerExists(listener)) {
		m_serverDB.getChannelListenerTable().addListener(listener);
	}

	WRAPPER_END
}

void DBWrapper::disableChannelListenerIfExists(unsigned serverID, const ServerUserInfo &userInfo,
											   const Channel &channel) {
	WRAPPER_BEGIN

	::msdb::DBChannelListener listener(serverID, channel.iId, userInfo.iId);

	if (m_serverDB.getChannelListenerTable().listenerExists(listener)) {
		listener = m_serverDB.getChannelListenerTable().getListenerDetails(listener);

		if (listener.enabled) {
			listener.enabled = false;
			m_serverDB.getChannelListenerTable().updateListener(listener);
		}
	}

	WRAPPER_END
}

void DBWrapper::deleteChannelListener(unsigned int serverID, const ServerUserInfo &userInfo, const Channel &channel) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);

	m_serverDB.getChannelListenerTable().removeListener(serverID, userInfo.iId, channel.iId);

	WRAPPER_END
}

void DBWrapper::loadChannelListenersOf(unsigned int serverID, const ServerUserInfo &userInfo,
									   ChannelListenerManager &manager) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);

	for (const ::msdb::DBChannelListener &currentListener :
		 m_serverDB.getChannelListenerTable().getListenersForUser(serverID, userInfo.iId)) {
		if (currentListener.enabled) {
			manager.addListener(userInfo.uiSession, currentListener.channelID);
			manager.setListenerVolumeAdjustment(userInfo.uiSession, currentListener.channelID,
												VolumeAdjustment::fromFactor(currentListener.volumeAdjustment));
		}
	}

	WRAPPER_END
}

void DBWrapper::storeChannelListenerVolume(unsigned int serverID, const ServerUserInfo &userInfo,
										   const Channel &channel, float volumeFactor) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);

	::msdb::DBChannelListener listener =
		m_serverDB.getChannelListenerTable().getListenerDetails(serverID, userInfo.iId, channel.iId);

	if (listener.volumeAdjustment != volumeFactor) {
		listener.volumeAdjustment = volumeFactor;
		m_serverDB.getChannelListenerTable().updateListener(listener);
	}

	WRAPPER_END
}

unsigned int DBWrapper::registerUser(unsigned int serverID, const ServerUserInfo &userInfo) {
	WRAPPER_BEGIN

	assert(userInfo.cChannel);

	::mdb::TransactionHolder transaction = m_serverDB.ensureTransaction();

	unsigned int userID = userInfo.iId < 0 ? m_serverDB.getUserTable().getFreeUserID(serverID) : userInfo.iId;

	::msdb::DBUser user(serverID, userID);

	m_serverDB.getUserTable().addUser(user, userInfo.qsName.toStdString());

	::msdb::DBUserData data;
	data.name          = userInfo.qsName.toStdString();
	data.lastChannelID = userInfo.cChannel->iId;
	if (!userInfo.qbaTexture.isEmpty()) {
		data.texture.resize(userInfo.qbaTexture.size());

		static_assert(sizeof(decltype(data.texture)::value_type) == sizeof(decltype(userInfo.qbaTexture)::value_type),
					  "Data types are not compatible");
		std::memcpy(data.texture.data(), reinterpret_cast< const std::uint8_t * >(userInfo.qbaTexture.data()),
					userInfo.qbaTexture.size());
	}

	setUserData(serverID, userID, data);


	std::vector< std::pair< unsigned int, std::string > > properties;
	properties.push_back(
		{ static_cast< unsigned int >(::msdb::UserProperty::CertificateHash), userInfo.qsHash.toStdString() });

	if (!userInfo.qslEmail.isEmpty()) {
		properties.push_back(
			{ static_cast< unsigned int >(::msdb::UserProperty::Email), userInfo.qslEmail.first().toStdString() });
	}

	if (!userInfo.qsComment.isEmpty()) {
		properties.push_back(
			{ static_cast< unsigned int >(::msdb::UserProperty::Comment), userInfo.qsComment.toStdString() });
	}

	setUserProperties(serverID, userID, properties);


	transaction.commit();

	return userID;

	WRAPPER_END
}

void DBWrapper::unregisterUser(unsigned int serverID, unsigned int userID) {
	WRAPPER_BEGIN

	::msdb::DBUser user(serverID, userID);
	m_serverDB.getUserTable().removeUser(user);

	WRAPPER_END
}

int DBWrapper::registeredUserNameToID(unsigned int serverID, const std::string &name) {
	WRAPPER_BEGIN

	boost::optional< unsigned int > id = m_serverDB.getUserTable().findUser(serverID, name, false);

	return id ? static_cast< int >(id.get()) : -1;

	WRAPPER_END
}

bool DBWrapper::registeredUserExists(unsigned int serverID, unsigned int userID) {
	WRAPPER_BEGIN

	::msdb::DBUser user(serverID, userID);

	return m_serverDB.getUserTable().userExists(user);

	WRAPPER_END
}

QMap< int, QString > DBWrapper::getRegisteredUserDetails(unsigned int serverID, unsigned int userID) {
	WRAPPER_BEGIN

	QMap< int, QString > details;

	::msdb::DBUser user(serverID, userID);
	::msdb::DBUserData userData = m_serverDB.getUserTable().getData(user);

	details.insert({ static_cast< int >(::msdb::UserProperty::Name) }, QString::fromStdString(userData.name));
	details.insert({ static_cast< int >(::msdb::UserProperty::LastActive) },
				   QDateTime::fromSecsSinceEpoch(::msdb::toEpochSeconds(userData.lastActive)).toString(Qt::ISODate));

	for (const std::pair< unsigned int, std::string > &currentProps : getUserProperties(serverID, userID)) {
		details.insert(static_cast< int >(currentProps.first), QString::fromStdString(currentProps.second));
	}

	return details;

	WRAPPER_END
}


void DBWrapper::addAllRegisteredUserInfoTo(std::vector< UserInfo > &userInfo, unsigned int serverID,
										   const std::string &nameFilter) {
	WRAPPER_BEGIN

	for (const ::msdb::DBUser &currentUser : m_serverDB.getUserTable().getRegisteredUsers(serverID, nameFilter)) {
		::msdb::DBUserData userData = m_serverDB.getUserTable().getData(currentUser);

		UserInfo info;
		info.name         = QString::fromStdString(userData.name);
		info.user_id      = currentUser.registeredUserID;
		info.last_active  = QDateTime::fromSecsSinceEpoch(::msdb::toEpochSeconds(userData.lastActive));
		info.last_channel = userData.lastChannelID;

		userInfo.push_back(std::move(info));
	}

	WRAPPER_END
}

void DBWrapper::setLastChannel(unsigned int serverID, const ServerUserInfo &userInfo) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);
	assert(userInfo.cChannel);

	::msdb::DBUser user(serverID, userInfo.iId);

	m_serverDB.getUserTable().setLastChannelID(user, userInfo.cChannel->iId);

	WRAPPER_END
}

int DBWrapper::getLastChannelID(unsigned int serverID, const ServerUserInfo &userInfo) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);

	::msdb::DBUser user(serverID, userInfo.iId);

	return m_serverDB.getUserTable().getData(user).lastChannelID;

	WRAPPER_END
}

QByteArray DBWrapper::getUserTexture(unsigned int serverID, const ServerUserInfo &userInfo) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);

	::msdb::DBUser user(serverID, userInfo.iId);
	::msdb::DBUserData data = m_serverDB.getUserTable().getData(user);

	QByteArray texture;
	if (!data.texture.empty()) {
		texture.resize(data.texture.size());

		std::memcpy(reinterpret_cast< std::uint8_t * >(texture.data()), data.texture.data(), data.texture.size());
	}

	if (texture.size() == 600 * 60 * 4) {
		texture = qCompress(texture);
	}

	return texture;

	WRAPPER_END
}

void DBWrapper::storeUserTexture(unsigned int serverID, const ServerUserInfo &userInfo) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);

	::msdb::DBUser user(serverID, userInfo.iId);
	::msdb::DBUserData data = m_serverDB.getUserTable().getData(user);

	QByteArray texture =
		userInfo.qbaTexture.size() == 600 * 60 * 4 ? qCompress(userInfo.qbaTexture) : userInfo.qbaTexture;

	data.texture.resize(texture.size());
	std::memcpy(data.texture.data(), reinterpret_cast< const std::uint8_t * >(texture.data()), texture.size());

	m_serverDB.getUserTable().updateData(user, data);

	WRAPPER_END
}

std::string DBWrapper::getUserProperty(unsigned int serverID, const ServerUserInfo &userInfo,
									   ::msdb::UserProperty property) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);

	::msdb::DBUser user(serverID, userInfo.iId);

	return m_serverDB.getUserPropertyTable().getProperty< std::string, false >(user, property, {});

	WRAPPER_END
}

void DBWrapper::storeUserProperty(unsigned int serverID, const ServerUserInfo &userInfo, ::msdb::UserProperty property,
								  const std::string &value) {
	WRAPPER_BEGIN

	assert(userInfo.iId >= 0);

	::msdb::DBUser user(serverID, userInfo.iId);

	if (value.empty()) {
		m_serverDB.getUserPropertyTable().clearProperty(user, property);
	} else {
		m_serverDB.getUserPropertyTable().setProperty(user, property, value);
	}

	WRAPPER_END
}

void DBWrapper::setUserProperties(unsigned int serverID, unsigned int userID,
								  const std::vector< std::pair< unsigned int, std::string > > &properties) {
	WRAPPER_BEGIN

	::msdb::DBUser user(serverID, userID);

	for (const std::pair< unsigned int, std::string > &current : properties) {
		assert(current.first <= std::numeric_limits< std::underlying_type<::msdb::UserProperty >::type >::max());

		// Note that this casting could potentially cause UserProperty to take on new values (not actually defined in
		// the enum). However, the properties are only stored as integers in the database anyway and since above
		// assertion has checked (at least in debug mode) that the used type to represent a UserProperty is large enough
		// to hold the value, this should be fine.
		// Ideally, this will be changed at some point, but for now it should work.
		::msdb::UserProperty property = static_cast<::msdb::UserProperty >(current.first);

		if (property == ::msdb::UserProperty::Name || property == ::msdb::UserProperty::kdfIterations
			|| property == ::msdb::UserProperty::LastActive || property == ::msdb::UserProperty::Password) {
			// These are all properties that are supposed to be stored in the user table rather than the user property
			// table We assume that the calling code has taken care of this and won't even pass those down here.
			assert(false && "These properties should have been processed separately, before calling this function");
			continue;
		}

		m_serverDB.getUserPropertyTable().setProperty(user, property, current.second);
	}

	WRAPPER_END
}

std::vector< std::pair< unsigned int, std::string > > DBWrapper::getUserProperties(unsigned int serverID,
																				   unsigned int userID) {
	WRAPPER_BEGIN

	std::vector< std::pair< unsigned int, std::string > > properties;

	::mdb::TransactionHolder transaction = m_serverDB.ensureTransaction();

	// Start with user properties that are stored in the user table itself
	::msdb::DBUser user(serverID, userID);
	::msdb::DBUserData userData = m_serverDB.getUserTable().getData(user);

	properties.push_back({ static_cast< unsigned int >(::msdb::UserProperty::Name), userData.name });
	properties.push_back({ static_cast< unsigned int >(::msdb::UserProperty::LastActive),
						   QDateTime::fromSecsSinceEpoch(::msdb::toEpochSeconds(userData.lastActive))
							   .toString(Qt::ISODate)
							   .toStdString() });
	// Note: we explicitly don't insert the password and kdfIterations property - those are secret and not handed out!

	// Fetch remaining properties (but only those that we know of)
	std::string email =
		m_serverDB.getUserPropertyTable().getProperty< std::string, false >(user, ::msdb::UserProperty::Email);
	if (!email.empty()) {
		properties.push_back({ static_cast< unsigned int >(::msdb::UserProperty::Email), std::move(email) });
	}

	std::string comment =
		m_serverDB.getUserPropertyTable().getProperty< std::string, false >(user, ::msdb::UserProperty::Comment);
	if (!comment.empty()) {
		properties.push_back({ static_cast< unsigned int >(::msdb::UserProperty::Comment), std::move(comment) });
	}

	std::string certHash = m_serverDB.getUserPropertyTable().getProperty< std::string, false >(
		user, ::msdb::UserProperty::CertificateHash);
	if (!certHash.empty()) {
		properties.push_back(
			{ static_cast< unsigned int >(::msdb::UserProperty::CertificateHash), std::move(certHash) });
	}

	transaction.commit();

	return properties;

	WRAPPER_END
}

std::string DBWrapper::getUserName(unsigned int serverID, unsigned int userID) {
	WRAPPER_BEGIN

	::msdb::DBUser user(serverID, userID);

	return m_serverDB.getUserTable().getData(user).name;

	WRAPPER_END
}

unsigned int DBWrapper::getNextAvailableUserID(unsigned int serverID) {
	WRAPPER_BEGIN

	return m_serverDB.getUserTable().getFreeUserID(serverID);

	WRAPPER_END
}

void DBWrapper::setUserData(unsigned int serverID, unsigned int userID, const ::msdb::DBUserData &data) {
	WRAPPER_BEGIN

	::msdb::DBUser user(serverID, userID);

	m_serverDB.getUserTable().updateData(user, data);

	WRAPPER_END
}