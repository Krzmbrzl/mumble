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

float dbToFactor(int dbAdjustment) {
	// dB formula: +6dB double the volume
	return std::pow(2.0f, dbAdjustment / 6.0f);
}

int factorToDb(float factor) {
	// dB formula: Doubling the volume corresponds to +6dB
	return static_cast< int >(std::round(std::log2(factor) * 6));
}

ListenerVolumeDialog::ListenerVolumeDialog(ClientUser *user, Channel *channel, QWidget *parent)
	: QDialog(parent), m_user(user), m_channel(channel) {
	assert(m_user->uiSession == Global::get().uiSession);

	setupUi(this);

	m_initialAdjustemt   = Global::get().channelListenerManager->getListenerLocalVolumeAdjustment(m_channel->iId);
	m_lastSentAdjustment = m_initialAdjustemt;

	volumeBox->setValue(factorToDb(m_initialAdjustemt));

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
	float factor = dbToFactor(volumeBox->value());

	ServerHandlerPtr handler = Global::get().sh;
	if (!handler) {
		return;
	}

	Global::get().channelListenerManager->setListenerLocalVolumeAdjustment(m_channel->iId, factor);
}

void ListenerVolumeDialog::reject() {
	ServerHandlerPtr handler = Global::get().sh;
	if (handler) {
		Global::get().channelListenerManager->setListenerLocalVolumeAdjustment(m_channel->iId, m_initialAdjustemt);
	}

	QDialog::reject();
}

void ListenerVolumeDialog::accept() {
	on_applyBtn_clicked();

	QDialog::accept();
}
