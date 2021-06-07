// Copyright 2020-2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "TalkingUISelection.h"
#include "MainWindow.h"
#include "UserModel.h"
#include "Global.h"

#include <QVariant>
#include <QWidget>

#include <QDebug>

TalkingUISelection::TalkingUISelection(QWidget *widget) : m_widget(widget) {
}


void TalkingUISelection::setActive(bool active) {
	if (m_widget) {
		m_widget->setProperty("selected", active);
		// Repolish the widget's style so that the new property can take effect
		m_widget->style()->unpolish(m_widget);
		m_widget->style()->polish(m_widget);
		m_widget->update();
	}
}

void TalkingUISelection::apply() {
	setActive(true);
}

void TalkingUISelection::discard() {
	setActive(false);
}

bool TalkingUISelection::operator==(const TalkingUISelection &other) const {
	return m_widget == other.m_widget;
}

bool TalkingUISelection::operator!=(const TalkingUISelection &other) const {
	return m_widget != other.m_widget;
}

bool TalkingUISelection::operator==(const QWidget *widget) const {
	return m_widget == widget;
}

bool TalkingUISelection::operator!=(const QWidget *widget) const {
	return m_widget != widget;
}


UserSelection::UserSelection(QWidget *widget, unsigned int userSession)
	: TalkingUISelection(widget), m_userSession(userSession) {
}

void UserSelection::syncToMainWindow() const {
	if (Global::get().mw && Global::get().mw->pmModel) {
		Global::get().mw->pmModel->setSelectedUser(m_userSession);
	}
}

std::unique_ptr< TalkingUISelection > UserSelection::cloneToHeap() const {
	return std::make_unique< UserSelection >(*this);
}



ChannelSelection::ChannelSelection(QWidget *widget, int channelID)
	: TalkingUISelection(widget), m_channelID(channelID) {
}

void ChannelSelection::syncToMainWindow() const {
	if (Global::get().mw && Global::get().mw->pmModel) {
		Global::get().mw->pmModel->setSelectedChannel(m_channelID);
	}
}

std::unique_ptr< TalkingUISelection > ChannelSelection::cloneToHeap() const {
	return std::make_unique< ChannelSelection >(*this);
}



ListenerSelection::ListenerSelection(QWidget *widget, unsigned int userSession, int channelID)
	: TalkingUISelection(widget), m_userSession(userSession), m_channelID(channelID) {
}

void ListenerSelection::syncToMainWindow() const {
	if (Global::get().mw && Global::get().mw->pmModel) {
		Global::get().mw->pmModel->setSelectedChannelListener(m_userSession, m_channelID);
	}
}

std::unique_ptr< TalkingUISelection > ListenerSelection::cloneToHeap() const {
	return std::make_unique< ListenerSelection >(*this);
}



LocalListenerSelection::LocalListenerSelection(QWidget *widget, int channelID)
	: ListenerSelection(widget, Global::get().uiSession, channelID) {
}

void LocalListenerSelection::setActive(bool active) {
	// We have to reset all local stylesheets here in order to make the transparent background color disappear
	// as that would prevent the theme's background color for the new active state to take effect (as the local
	// change would overwrite it).
	m_widget->setStyleSheet("");

	ListenerSelection::setActive(active);

	if (m_widget && !active) {
		// clear the property again in order to avoid a permanent background color for the listener
		// icon in the GroupBox
		// (Setting to an invalid QVariant clears the property)
		m_widget->setProperty("selected", QVariant());
		// As the icon was previously assigned a background color, it would default to white when we
		// remove the property-based background color again. We want it to be transparent though and
		// thus we have to explicitly make the background color transparent.
		m_widget->setStyleSheet("background-color: transparent");
		m_widget->style()->unpolish(m_widget);
	}
}

std::unique_ptr< TalkingUISelection > LocalListenerSelection::cloneToHeap() const {
	return std::make_unique< LocalListenerSelection >(*this);
}



void EmptySelection::syncToMainWindow() const {
	// Do nothing
}

std::unique_ptr< TalkingUISelection > EmptySelection::cloneToHeap() const {
	return std::make_unique< EmptySelection >(*this);
}
