// Copyright 2005-2019 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_PLUGININSTALLER_H_
#define MUMBLE_MUMBLE_PLUGININSTALLER_H_


#include <QtCore/QFileInfo>
#include <QtCore/QException>

#include "Plugin.h"

#include "ui_PluginInstaller.h"

class PluginInstallException : public QException {
	protected:
		QString msg;
	public:
		PluginInstallException(const QString& msg);

		QString getMessage() const;
};

class PluginInstaller : public QDialog, public Ui::PluginInstaller {
	private:
		Q_OBJECT;
		Q_DISABLE_COPY(PluginInstaller);
	protected:
		QFileInfo pluginArchive;
		Plugin *plugin;
		QFileInfo pluginSource;
		QFileInfo pluginDestination;
		bool copyPlugin;

		void init();
	public:
		static const QString pluginFileExtension;

		static bool canBePluginFile(const QFileInfo& fileInfo) noexcept;

		PluginInstaller(const QFileInfo& fileInfo, QWidget *p = nullptr);
		~PluginInstaller();

		void install() const;

	public slots:
		void on_qpbYesClicked();
		void on_qpbNoClicked();
};

#endif // MUMBLE_MUMBLE_PLUGININSTALLER_H_
