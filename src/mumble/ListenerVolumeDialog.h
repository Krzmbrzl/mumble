// Copyright 2020-2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_LISTENERVOLUME_H_
#define MUMBLE_MUMBLE_LISTENERVOLUME_H_

#include "VolumeAdjustment.h"

#include "ui_ListenerVolumeDialog.h"

class ClientUser;
class Channel;

/// The dialog to configure the local volume adjustment for a channel listener. Therefore
/// this dialog can be used to tune the volume of audio streams one receives via the listening
/// feature.
class ListenerVolumeDialog : public QDialog, private Ui::ListenerVolumeDialog {
private:
	Q_OBJECT
	Q_DISABLE_COPY(ListenerVolumeDialog);

protected:
	/// The user belonging to the listener proxy this dialog has been invoked on
	ClientUser *m_user;
	/// The channel of the listener proxy this dialog has been invoked on
	Channel *m_channel;
	/// The volume adjustment that was set before this dialog opened
	VolumeAdjustment m_initialAdjustemt;
	/// The volume adjustment that has been sent to the server most recently
	VolumeAdjustment m_lastSentAdjustment;

public slots:
	void on_volumeSlider_valueChanged(int value);
	void on_volumeBox_valueChanged(int value);
	void on_okBtn_clicked();
	void on_cancelBtn_clicked();
	void on_applyBtn_clicked();
	void on_resetBtn_clicked();
	void reject();
	void accept();

public:
	ListenerVolumeDialog(ClientUser *user, Channel *channel, QWidget *parent = nullptr);
};

#endif
