// Copyright 2005-2020 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_USERMODEL_H_
#define MUMBLE_MUMBLE_USERMODEL_H_

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtGui/QIcon>

class User;
class ClientUser;
class Channel;

struct ModelItem Q_DECL_FINAL {
	friend class UserModel;

private:
	Q_DISABLE_COPY(ModelItem)
public:
	Channel *cChan;
	ClientUser *pUser;

	bool isListener;

	bool bCommentSeen;

	ModelItem *parent;
	QList<ModelItem *> qlChildren;
	QList<ModelItem *> qlHiddenChildren;
	/// Number of users in this channel (recursive)
	int iUsers;

	static QHash <Channel *, ModelItem *> c_qhChannels;
	static QHash <ClientUser *, ModelItem *> c_qhUsers;
	static QHash <ClientUser *, QList<ModelItem *>> s_userProxies;
	static bool bUsersTop;

	ModelItem(Channel *c);
	ModelItem(ClientUser *p, bool isListener = false);
	ModelItem(ModelItem *);
	~ModelItem();

	ModelItem *child(int idx) const;

	bool validRow(int idx) const;
	ClientUser *userAt(int idx) const;
	Channel *channelAt(int idx) const;
	int rowOf(Channel *c) const;
	int rowOf(ClientUser *p, bool isListener) const;
	int rowOfSelf() const;
	int rows() const;
	int insertIndex(Channel *c) const;
	int insertIndex(ClientUser *p, bool isListener = false) const;
	QString hash() const;
	void wipe();
};

class UserModel : public QAbstractItemModel {
		friend struct ModelItem;
		friend class UserView;
	private:
		Q_OBJECT
		Q_DISABLE_COPY(UserModel)
	protected:
		QIcon qiTalkingOn, qiTalkingWhisper, qiTalkingShout, qiTalkingOff;
		QIcon qiMutedPushToMute, qiMutedSelf, qiMutedServer, qiMutedLocal, qiIgnoredLocal, qiMutedSuppressed;
		QIcon qiPrioritySpeaker;
		QIcon qiRecording;
		QIcon qiDeafenedSelf, qiDeafenedServer;
		QIcon qiAuthenticated, qiChannel, qiLinkedChannel, qiActiveChannel;
		QIcon qiFriend;
		QIcon qiComment, qiCommentSeen, qiFilter;
		QIcon qiLock_locked, qiLock_unlocked;
		QIcon qiEar;
		ModelItem *miRoot;
		QSet<Channel *> qsLinked;
		QMap<QString, ClientUser *> qmHashes;

		bool bClicked;

		void recursiveClone(const ModelItem *old, ModelItem *item, QModelIndexList &from, QModelIndexList &to);
		ModelItem *moveItem(ModelItem *oldparent, ModelItem *newparent, ModelItem *item);

		QString stringIndex(const QModelIndex &index) const;

		/// @returns The QModelIndex that is currently selected. If there is no selection, the returned index
		/// 	is invalid.
		QModelIndex getSelectedIndex() const;

		/// Removes the given user as a listener to the given channel
		///
		/// @param item A pointer to the listener's ModelItem that shall be removed
		/// @param citem A pointer to the ModelItem that represents the channel the listener
		/// 	is in. The listener has to be a direct child of this item. If this is nullptr,
		/// 	the parent of the provided item is used directly.
		void removeChannelListener(ModelItem *item, ModelItem *citem = nullptr);
	public:
		UserModel(QObject *parent = 0);
		~UserModel() Q_DECL_OVERRIDE;

		QModelIndex index(ClientUser *, int column = 0) const;
		QModelIndex index(Channel *, int column = 0) const;
		QModelIndex index(ModelItem *) const;

		QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
		Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
		Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;
		QStringList mimeTypes() const Q_DECL_OVERRIDE;
		QMimeData *mimeData(const QModelIndexList &idx) const Q_DECL_OVERRIDE;
		bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex & parent) Q_DECL_OVERRIDE;

		ClientUser *addUser(unsigned int id, const QString &name);
		ClientUser *getUser(const QModelIndex &idx) const;
		ClientUser *getUser(const QString &hash) const;
		/// @returns A pointer to the currently selected User or nullptr if there is none
		ClientUser *getSelectedUser() const;
		/// Sets the selection to the User with the given session
		///
		/// @param session The session ID of the respective User
		void setSelectedUser(unsigned int session);

		Channel *addChannel(int id, Channel *p, const QString &name);
		Channel *getChannel(const QModelIndex &idx) const;
		/// @returns A pointer to the currently selected Channel or nullptr if there is none
		Channel *getSelectedChannel() const;
		/// Sets the selection to the Channel with the given ID
		///
		/// @param session The ID of the respective Channel
		void setSelectedChannel(int id);

		/// Adds the guven user as a listener to the given channel
		///
		/// @param p A pointer to the user
		/// @param c A pointer to the channel
		void addChannelListener(ClientUser *p, Channel *c);
		/// Removes the guven user as a listener to the given channel
		///
		/// @param p A pointer to the user
		/// @param c A pointer to the channel. If this is nullptr, then all listeners
		/// 	for the given user are removed (from all channels).
		void removeChannelListener(ClientUser *p, Channel *c = nullptr);
		/// @param idx The QModelIndex to check
		/// @returns Whether the ModelItem associated with the given index is a listener-proxy
		bool isChannelListener(const QModelIndex &idx) const;

		Channel *getSubChannel(Channel *p, int idx) const;

		void renameUser(ClientUser *p, const QString &name);
		void renameChannel(Channel *c, const QString &name);
		void repositionChannel(Channel *c, const int position);
		void setUserId(ClientUser *p, int id);
		void setHash(ClientUser *p, const QString &hash);
		void setFriendName(ClientUser *p, const QString &name);
		void setComment(ClientUser *p, const QString &comment);
		void setCommentHash(ClientUser *p, const QByteArray &hash);
		void seenComment(const QModelIndex &idx);

		void moveUser(ClientUser *p, Channel *c);
		void moveChannel(Channel *c, Channel *p);
		void setComment(Channel *c, const QString &comment);
		void setCommentHash(Channel *c, const QByteArray &hash);

		void removeUser(ClientUser *p);
		bool removeChannel(Channel *c, const bool onlyIfUnoccupied = false);

		void linkChannels(Channel *c, QList<Channel *> links);
		void unlinkChannels(Channel *c, QList<Channel *> links);
		void unlinkAll(Channel *c);

		void removeAll();

		void expandAll(Channel *c);
		void collapseEmpty(Channel *c);

		QVariant otherRoles(const QModelIndex &idx, int role) const;

		unsigned int uiSessionComment;
		int iChannelDescription;
	public slots:
		/// Invalidates the model data of the ClientUser triggering this slot.
		void userStateChanged();
		void ensureSelfVisible();
		void recheckLinks();
		void updateOverlay() const;
		void toggleChannelFiltered(Channel *c);
};

#endif
