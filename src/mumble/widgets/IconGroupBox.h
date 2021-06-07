// Copyright 2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_WIDGETS_ICONGROUPBOX_H_
#define MUMBLE_MUMBLE_WIDGETS_ICONGROUPBOX_H_

#include <QFrame>
#include <QIcon>

class QLabel;
class QWidget;
class QString;
class QIcon;

class IconGroupBox : public QFrame {
	Q_OBJECT;
	Q_DISABLE_COPY(IconGroupBox);

public:
	IconGroupBox(const QString &title = "", const QIcon &icon = {}, QWidget *parent = nullptr);
	~IconGroupBox();

	void setTitle(const QString &title);
	QString title() const;

	void setIcon(const QIcon &icon);
	QIcon icon() const;
	void clearIcon();

	QWidget *getIconWidget();
	QWidget *getTitleWidget();

protected:
	QIcon m_icon;
	QWidget *m_header;
	QLabel *m_iconLabel;
	QLabel *m_titleLabel;

	void setupUI(const QString &title);

	void updateHeaderVisibility();
};

#endif // MUMBLE_MUMBLE_WIDGETS_ICONGROUPBOX_H_
