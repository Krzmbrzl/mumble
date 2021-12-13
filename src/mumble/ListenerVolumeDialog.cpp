// Copyright 2020-2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "ListenerVolumeDialog.h"
#include "Channel.h"
#include "ChannelListenerManager.h"
#include "ClientUser.h"
#include "Mumble.pb.h"
#include "MumbleProtocol.h"
#include "ServerHandler.h"
#include "Global.h"

#include <QtWidgets/QPushButton>

#include <cassert>
#include <cmath>

ListenerVolumeDialog::ListenerVolumeDialog(ClientUser *user, Channel *channel, QWidget *parent)
	: QDialog(parent), m_user(user), m_channel(channel) {
	assert(m_user->uiSession == Global::get().uiSession);

	setupUi(this);

	m_initialAdjustemt =
		Global::get().channelListenerManager->getListenerVolumeAdjustment(m_user->uiSession, m_channel->iId);
	m_lastSentAdjustment = m_initialAdjustemt;

	volumeBox->setValue(m_initialAdjustemt.dbAdjustment);

	setWindowTitle(tr("Adjusting local volume for listening to %1").arg(channel->qsName));
}

void ListenerVolumeDialog::on_volumeSlider_valueChanged(int value) {
	volumeBox->setValue(value);
}

void ListenerVolumeDialog::on_volumeBox_valueChanged(int value) {
	volumeSlider->setValue(value);
}

void ListenerVolumeDialog::on_okBtn_clicked() {
	ListenerVolumeDialog::accept();
}

void ListenerVolumeDialog::on_cancelBtn_clicked() {
	reject();
}

void ListenerVolumeDialog::on_resetBtn_clicked() {
	volumeBox->setValue(0);
}

void ListenerVolumeDialog::on_applyBtn_clicked() {
	VolumeAdjustment adjustment = VolumeAdjustment::fromDBAdjustment(volumeBox->value());

	ServerHandlerPtr handler = Global::get().sh;
	if (!handler) {
		return;
	}

	if (handler->uiVersion >= Mumble::Protocol::PROTOBUF_INTRODUCTION_VERSION) {
		// Volume adjustments for listeners are handled on the server, since the protocol supports attaching volume
		// adjustments
		m_lastSentAdjustment = adjustment;

		MumbleProto::UserState mpus;
		mpus.set_session(m_user->uiSession);

		MumbleProto::UserState::VolumeAdjustment *adjustmentMsg = mpus.add_listening_volume_adjustment();
		adjustmentMsg->set_listening_channel(m_channel->iId);
		adjustmentMsg->set_volume_adjustment(adjustment.factor);

		handler->sendMessage(mpus);
	} else {
		// Before that the adjustments are handled locally
		Global::get().channelListenerManager->setListenerVolumeAdjustment(m_user->uiSession, m_channel->iId,
																		  adjustment);
	}
}

void ListenerVolumeDialog::reject() {
	ServerHandlerPtr handler = Global::get().sh;
	if (handler) {
		if (handler->uiVersion >= Mumble::Protocol::PROTOBUF_INTRODUCTION_VERSION) {
			if (m_initialAdjustemt != m_lastSentAdjustment) {
				MumbleProto::UserState mpus;
				mpus.set_session(m_user->uiSession);

				MumbleProto::UserState::VolumeAdjustment *adjustment = mpus.add_listening_volume_adjustment();
				adjustment->set_listening_channel(m_channel->iId);
				adjustment->set_volume_adjustment(m_initialAdjustemt.factor);

				handler->sendMessage(mpus);
			}
		} else {
			Global::get().channelListenerManager->setListenerVolumeAdjustment(m_user->uiSession, m_channel->iId,
																			  m_initialAdjustemt);
		}
	}

	QDialog::reject();
}

void ListenerVolumeDialog::accept() {
	on_applyBtn_clicked();

	QDialog::accept();
}
