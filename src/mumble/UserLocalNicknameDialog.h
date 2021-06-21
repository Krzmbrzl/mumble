// Copyright 2020-2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_USERNICKNAME_H_
#define MUMBLE_MUMBLE_USERNICKNAME_H_

#include "QtUtils.h"

#include <memory>
#include <unordered_map>

#include "ClientUser.h"
#include "ui_UserLocalNicknameDialog.h"

class UserLocalNicknameDialog;

using NicknameDialogMap =
	std::unordered_map< unsigned int, Mumble::QtUtils::qobject_unique_ptr< UserLocalNicknameDialog > >;

class UserLocalNicknameDialog : public QDialog, private Ui::UserLocalNicknameDialog {
	Q_OBJECT
	Q_DISABLE_COPY(UserLocalNicknameDialog);

	/// The session ID for the user that the dialog is changing the nickname for.
	unsigned int m_clientSession;

	/// The user's original nickname when entering the dialog.
	QString m_originalNickname;
	NicknameDialogMap &m_qmUserNicknameTracker;

public slots:
	void closeEvent(QCloseEvent *event);
	void on_qleUserLocalNickname_textChanged(const QString &text);
	void on_qbbUserLocalNickname_clicked(QAbstractButton *b);
	void reject();

public:
	static void present(unsigned int sessionId, NicknameDialogMap &qmUserNicknameTracker);
	UserLocalNicknameDialog(unsigned int sessionId, NicknameDialogMap &qmUserNicknameTracker);
};

#endif
