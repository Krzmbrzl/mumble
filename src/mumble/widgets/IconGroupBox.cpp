// Copyright 2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "widgets/IconGroupBox.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

IconGroupBox::IconGroupBox(const QString &title, const QIcon &icon, QWidget *parent) : QFrame(parent), m_icon(icon) {
	setupUI(title);
}

IconGroupBox::~IconGroupBox() {
}

void IconGroupBox::setTitle(const QString &title) {
	m_titleLabel->setText(title);

	updateHeaderVisibility();
}

QString IconGroupBox::title() const {
	return m_titleLabel->text();
}

void IconGroupBox::setIcon(const QIcon &icon) {
	m_icon = icon;

	if (icon.isNull()) {
		m_iconLabel->setPixmap(QPixmap());
	} else {
		QFontMetrics metrics(font());
		m_iconLabel->setPixmap(m_icon.pixmap(metrics.height(), QIcon::Mode::Normal));
	}

	updateHeaderVisibility();
}

QIcon IconGroupBox::icon() const {
	return m_icon;
}

void IconGroupBox::clearIcon() {
	// Setting to null icon clears the icon
	setIcon(QIcon());
}

QWidget *IconGroupBox::getIconWidget() {
	return m_iconLabel;
}

QWidget *IconGroupBox::getTitleWidget() {
	return m_titleLabel;
}

void IconGroupBox::setupUI(const QString &title) {
	QVBoxLayout *layout = new QVBoxLayout();

	QHBoxLayout *headerLayout = new QHBoxLayout();
	headerLayout->setContentsMargins(0, 0, 0, 0);

	m_header = new QWidget();
	m_header->setLayout(headerLayout);
	layout->addWidget(m_header);

	m_iconLabel = new QLabel();
	m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	headerLayout->addWidget(m_iconLabel);

	m_titleLabel = new QLabel();
	m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_titleLabel->setText(title);
	headerLayout->addWidget(m_titleLabel);

	setLayout(layout);

	// Set the icon at the end of the current event loop. We can't do it immediately since it takes some
	// time for the widget to actually apply its font properly and we need the font size in order to
	// determine the icon size.
	// By parenting the timer to this object, we make sure that the timer gets killed when this object gets
	// destroyed.
	QTimer *timer = new QTimer(this);
	timer->setSingleShot(true);
	QObject::connect(timer, &QTimer::timeout, [&]() { setIcon(m_icon); });
	timer->start(0);
}

void IconGroupBox::updateHeaderVisibility() {
	// Hide the entire header if there is not icon and no title. Otherwise show it
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	m_header->setVisible(!m_iconLabel->pixmap(Qt::ReturnByValue).isNull() || !m_titleLabel->text().isEmpty());
	m_iconLabel->setVisible(!m_iconLabel->pixmap(Qt::ReturnByValue).isNull());
#else
	// The pointer-returning overload of the pixmap() function got deprecated in Qt 5.15
	m_header->setVisible(!m_iconLabel->pixmap() || !m_titleLabel->text().isEmpty());
	m_iconLabel->setVisible(!m_iconLabel->pixmap());
#endif

	m_titleLabel->setVisible(!m_titleLabel->text().isEmpty());
}
