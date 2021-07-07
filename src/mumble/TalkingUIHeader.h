// Copyright 2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_TALKINGUIHEADER_H_
#define MUMBLE_MUMBLE_TALKINGUIHEADER_H_

#include "MultiStyleWidgetWrapper.h"
#include "Settings.h"
#include "TalkingUIComponent.h"
#include "TalkingUIEntry.h"

#include <QTimer>

class QFrame;
class QWidget;
class QLabel;
class Channel;

class WhisperTargetDisplay : public QWidget {
public:
	WhisperTargetDisplay(QWidget *parent = nullptr);

	void setIconSize(unsigned int size);

public slots:
	void on_voiceTargetChanged(int target);

protected:
	QLabel *m_targetString;
	QLabel *m_icons;
	QLabel *m_group;
	MultiStyleWidgetWrapper m_targetStyle;
	unsigned int m_iconSize = 0;

	void setupUI();
};

class TalkingUIHeader : public TalkingUIComponent {
public:
	TalkingUIHeader(QWidget *parent = nullptr);
	virtual ~TalkingUIHeader();

	virtual QWidget *getWidget() override;
	virtual const QWidget *getWidget() const override;
	virtual MultiStyleWidgetWrapper &getStylableWidget() override;

	QWidget *getUserNameWidget();
	QWidget *getTalkingIconWidget();
	QWidget *getChannelNameWidget();

	void on_serverSynchronized();
	void on_serverDisconnected();
	void on_channelChanged(const Channel *channel);
	void updateStatusIcons(const TalkingUIUser::UserStatus &status);
	void setIconSize(unsigned int size);
	void setTalkingState(Settings::TalkState state);

protected:
	QFrame *m_container;
	QLabel *m_notConnectedMsg;
	QWidget *m_infoBox;
	QLabel *m_talkIcon;
	QLabel *m_userName;
	QLabel *m_statusIcons;
	QLabel *m_channelName;
	WhisperTargetDisplay *m_whisperTargetDisplay;
	MultiStyleWidgetWrapper m_containerStyleWrapper;
	unsigned int m_iconSize;
	QTimer m_timer;

	void setupUI();
};

#endif // MUMBLE_MUMBLE_TALKINGUIHEADER_H_
