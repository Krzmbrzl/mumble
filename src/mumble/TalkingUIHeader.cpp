// Copyright 2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "TalkingUIHeader.h"
#include "Channel.h"
#include "ClientUser.h"
#include "Global.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QSizePolicy>
#include <QVBoxLayout>

TalkingUIHeader::TalkingUIHeader(QWidget *parent)
	: m_container(new QFrame(parent)), m_notConnectedMsg(new QLabel(m_container)), m_infoBox(new QWidget(m_container)),
	  m_talkIcon(new QLabel(m_infoBox)), m_userName(new QLabel(m_infoBox)), m_statusIcons(new QLabel(m_infoBox)),
	  m_channelName(new QLabel(m_infoBox)), m_containerStyleWrapper(m_container) {
	setupUI();

	// init size for talking icon to FontSize
	m_iconSize = QFontMetrics(m_container->font()).height();

	m_timer.setSingleShot(true);
	QObject::connect(&m_timer, &QTimer::timeout, [&]() {
		// Update the size again at the end of the current event loop iteration as the final font size will only
		// be available then.
		m_iconSize = QFontMetrics(m_container->font()).height();
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
