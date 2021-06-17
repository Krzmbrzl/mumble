// Copyright 2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "AudienceDialog.h"
#include "ClientUser.h"

AudienceDialog::AudienceDialog(const QVector< unsigned int > &sessions, QWidget *parent) : QDialog(parent) {
	setupUi(this);

	for (unsigned int currentSession : sessions) {
		const ClientUser *user = ClientUser::get(currentSession);

		if (!user) {
			continue;
		}

		userList->addItem(user->qsName);
	}

	userList->sortItems();

	qlExplanation->setText(tr("Users that will hear you if you start talking now (%1):").arg(sessions.size()));
}
