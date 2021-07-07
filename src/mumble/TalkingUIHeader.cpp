// Copyright 2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "TalkingUIHeader.h"
#include "Channel.h"
#include "ClientUser.h"
#include "MainWindow.h"
#include "Settings.h"
#include "UserModel.h"
#include "Global.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QPainter>
#include <QSizePolicy>
#include <QVBoxLayout>

WhisperTargetDisplay::WhisperTargetDisplay(QWidget *parent)
	: QWidget(parent), m_targetString(new QLabel(this)), m_icons(new QLabel(this)), m_group(new QLabel(this)) {
	setupUI();

	QObject::connect(Global::get().mw, &MainWindow::voiceTargetChanged, this,
					 &WhisperTargetDisplay::on_voiceTargetChanged);
}

void WhisperTargetDisplay::setIconSize(unsigned int size) {
	m_iconSize = size;
}

void WhisperTargetDisplay::on_voiceTargetChanged(int targetID) {
	if (targetID <= 0) {
		// target == 0 means direct speach (no whisper/shout in progress) and target < 0
		// means invalid target
		hide();
		return;
	}

	const QList< ShortcutTarget > &targets = Global::get().mw->voiceTargetsFor(targetID);

	if (targets.empty()) {
		return;
	}

	if (targets.size() > 1) {
		m_targetString->setText(tr("→ <i>%1 targets</i>").arg(targets.size()));
	} else {
		const ShortcutTarget &target = targets[0];

		const Channel *targetChannel = nullptr;
		const ClientUser *targetUser = nullptr;
		bool more                    = false;
		if (target.bCurrentSelection) {
			targetChannel = Global::get().mw->pmModel->getSelectedChannel();
			targetUser    = Global::get().mw->pmModel->getSelectedUser();
		} else if (target.bUsers) {
			// Whisper to users
			if (target.qlSessions.size() > 0) {
				// Pick first user as "representative"
				targetUser = ClientUser::get(target.qlSessions[0]);

				more = target.qlSessions.size() > 1;
			}
		} else {
			// Shout to channel
			targetChannel = Channel::get(target.iChannel);
		}

		QString displayString;
		QString tooltip;
		if (targetUser) {
			displayString = targetUser->qsName;
			if (more) {
				displayString += tr(" & …");
			}

			for (unsigned int session : target.qlSessions) {
				const ClientUser *user = ClientUser::get(session);

				if (user) {
					if (tooltip.isEmpty()) {
						tooltip = user->qsName;
					} else {
						tooltip = tooltip + ", " + user->qsName;
					}
				}
			}

			m_group->clear();
		} else if (targetChannel) {
			displayString = targetChannel->qsName;

			if (target.qsGroup.isEmpty()) {
				m_group->clear();
			} else {
				m_group->setText(tr("(%1)").arg(target.qsGroup));
			}

			// The tooltip is going to be the full channel path starting from root
			tooltip               = targetChannel->qsName;
			const Channel *parent = targetChannel->cParent;
			while (parent) {
				tooltip = parent->qsName + Global::get().s.qsHierarchyChannelSeparator + tooltip;

				parent = parent->cParent;
			}
		}

		m_targetString->setText(tr("→ %1").arg(displayString));
		m_targetString->setToolTip(tooltip);

		static const QIcon s_linkIcon     = QIcon(QLatin1String("skin:channel_linked.svg"));
		static const QIcon s_avatarIcon   = QIcon(QLatin1String("skin:talking_off.svg"));
		static const QIcon s_childrenIcon = QIcon(QLatin1String("skin:channel_tree.svg"));

		std::vector< std::reference_wrapper< const QIcon > > icons;

		if (target.bUsers) {
			icons.push_back(s_avatarIcon);
		} else {
			if (target.bChildren) {
				icons.push_back(s_childrenIcon);
			}
			if (target.bLinks) {
				icons.push_back(s_linkIcon);
			}
		}

		if (icons.empty()) {
			m_icons->clear();
		} else {
			const QSize size(m_iconSize * static_cast< int >(icons.size()), m_iconSize);
			QPixmap pixmap(size);
			pixmap.fill(Qt::transparent);

			QPainter painter(&pixmap);
			for (int i = 0; i < static_cast< int >(icons.size()); i++) {
				painter.drawPixmap(i * m_iconSize, 0,
								   icons[i].get().pixmap(QSize(m_iconSize, m_iconSize), QIcon::Normal, QIcon::On));
			}

			m_icons->setPixmap(pixmap);
		}
	}

	show();
}

void WhisperTargetDisplay::setupUI() {
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setAlignment(Qt::AlignLeft);

	layout->addWidget(m_targetString);
	layout->addWidget(m_icons);
	layout->addWidget(m_group);

	setLayout(layout);
}


TalkingUIHeader::TalkingUIHeader(QWidget *parent)
	: m_container(new QFrame(parent)), m_notConnectedMsg(new QLabel(m_container)), m_infoBox(new QWidget(m_container)),
	  m_talkIcon(new QLabel(m_infoBox)), m_userName(new QLabel(m_infoBox)), m_statusIcons(new QLabel(m_infoBox)),
	  m_channelName(new QLabel(m_infoBox)), m_whisperTargetDisplay(new WhisperTargetDisplay(m_container)),
	  m_containerStyleWrapper(m_container) {
	setupUI();

	// init size for talking icon to FontSize
	setIconSize(QFontMetrics(m_container->font()).height());

	m_timer.setSingleShot(true);
	QObject::connect(&m_timer, &QTimer::timeout, [&]() {
		// Update the size again at the end of the current event loop iteration as the final font size will only
		// be available then.
		setIconSize(QFontMetrics(m_container->font()).height());
	});
	m_timer.start(0);
}

TalkingUIHeader::~TalkingUIHeader() {
	m_timer.stop();
	m_container->deleteLater();
}

QWidget *TalkingUIHeader::getWidget() {
	return m_container;
}

const QWidget *TalkingUIHeader::getWidget() const {
	return m_container;
}

MultiStyleWidgetWrapper &TalkingUIHeader::getStylableWidget() {
	return m_containerStyleWrapper;
}

QWidget *TalkingUIHeader::getUserNameWidget() {
	return m_userName;
}

QWidget *TalkingUIHeader::getTalkingIconWidget() {
	return m_talkIcon;
}

QWidget *TalkingUIHeader::getChannelNameWidget() {
	return m_channelName;
}

void TalkingUIHeader::on_serverSynchronized() {
	const ClientUser *self = ClientUser::get(Global::get().uiSession);

	if (!self) {
		qWarning("TalkingUIHeader.cpp: Can't find local user");
		return;
	}

	m_userName->setText(QString::fromLatin1("<b>%1</b>").arg(self->qsName));
	m_channelName->setText(self->cChannel->qsName);

	setTalkingState(Settings::Passive);

	m_notConnectedMsg->hide();
	m_infoBox->show();
}

void TalkingUIHeader::on_serverDisconnected() {
	m_infoBox->hide();
	m_notConnectedMsg->show();
}

void TalkingUIHeader::on_channelChanged(const Channel *channel) {
	if (Global::get().uiSession > 0) {
		// Only access the channel object if we are actually still connected to a server (indicated by
		// a session ID > 0) as this can otherwise lead to a SegFault.
		m_channelName->setText(channel->qsName);
	}
}

void TalkingUIHeader::updateStatusIcons(const TalkingUIUser::UserStatus &status) {
	if (TalkingUIUser::paintStatusIcons(m_statusIcons, status, m_iconSize) > 0) {
		m_statusIcons->show();
	} else {
		m_statusIcons->hide();
	}
}

void TalkingUIHeader::setIconSize(unsigned int size) {
	m_iconSize = size;
	m_whisperTargetDisplay->setIconSize(size);
}

void TalkingUIHeader::setTalkingState(Settings::TalkState state) {
	m_talkIcon->setPixmap(
		TalkingUIUser::getTalkingIcon(state).pixmap(QSize(m_iconSize, m_iconSize), QIcon::Normal, QIcon::On));
}


void TalkingUIHeader::setupUI() {
	QVBoxLayout *containerLayout = new QVBoxLayout();
	containerLayout->setContentsMargins(0, 0, 0, 0);
	m_container->setLayout(containerLayout);

	m_notConnectedMsg->setText(QObject::tr("Not connected"));
	m_notConnectedMsg->setMargin(5);
	containerLayout->addWidget(m_notConnectedMsg);

	QVBoxLayout *infoBoxLayout = new QVBoxLayout();
	m_infoBox->setLayout(infoBoxLayout);
	containerLayout->addWidget(m_infoBox);


	// User line
	QHBoxLayout *userLine = new QHBoxLayout();
	userLine->setAlignment(Qt::AlignLeft);
	infoBoxLayout->addLayout(userLine);

	m_talkIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	userLine->addWidget(m_talkIcon);

	m_userName->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	userLine->addWidget(m_userName);

	userLine->addStretch();

	QHBoxLayout *iconLayout = new QHBoxLayout();
	userLine->addLayout(iconLayout);

	iconLayout->addWidget(m_statusIcons);
	// Hide by default
	m_statusIcons->hide();


	m_whisperTargetDisplay->hide();
	infoBoxLayout->addWidget(m_whisperTargetDisplay);


	// Channel line
	QHBoxLayout *channelLine = new QHBoxLayout();
	channelLine->setAlignment(Qt::AlignLeft);
	infoBoxLayout->addLayout(channelLine);

	m_channelName->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	channelLine->addWidget(m_channelName);


	// Hide info box by default
	m_infoBox->hide();

	// Make user and channel name participate in the selection in the TalkingUI
	m_userName->setProperty("selected", false);
	m_channelName->setProperty("selected", false);

	// Add a bit of padding around user and channel name
	m_userName->setContentsMargins(2, 1, 2, 1);
	m_channelName->setContentsMargins(2, 1, 2, 1);
}
