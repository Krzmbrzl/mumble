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

/// An exception thrown by the PluginInstaller
class PluginInstallException : public QException {
	protected:
		/// The exception's message
		QString msg;
	public:
		/// @param msg The message stating why this exception has been thrown
		PluginInstallException(const QString& msg);

		/// @returns This exception's message
		QString getMessage() const;
};

/// The PluginInstaller can be used to install plugins into Mumble. It verifies that the respective
/// plugin is functional and will automatiacally copy/move the plugin library to the respective
/// directory on the FileSystem.
class PluginInstaller : public QDialog, public Ui::PluginInstaller {
	private:
		Q_OBJECT;
		Q_DISABLE_COPY(PluginInstaller);
	protected:
		/// The file the installer has been invoked on
		QFileInfo pluginArchive;
		/// A pointer to the plugin instance created from the plugin library that shall be installed
		Plugin *plugin;
		/// The actual plugin library file
		QFileInfo pluginSource;
		/// The destinaton file to which the plugin library shall be copied
		QFileInfo pluginDestination;
		/// A flag indicating that the plugin library shall be copied instead of moved in order
		/// to install it.
		bool copyPlugin;

		/// Initializes this installer by processing the provided plugin source and filling all
		/// internal fields. This function is called from the constructor.
		///
		/// @throws PluginInstallException If something isn't right or goes wrong
		void init();
	public:
		/// The "special" file-extension associated with Mumble plugins
		static const QString pluginFileExtension;

		/// A helper function checking whether the provided file could be a plugin source
		///
		/// @param fileInfo The file to check
		/// @returns Whether the provided file could (!) be a plugin source
		static bool canBePluginFile(const QFileInfo& fileInfo) noexcept;

		/// @param fileInfo The plugin source to process
		///
		/// @throws PluginInstallException If something isn't right or goes wrong
		PluginInstaller(const QFileInfo& fileInfo, QWidget *p = nullptr);
		~PluginInstaller();

		/// Performs the actual installation (moving/copying of the library) of the plugin
		void install() const;

	public slots:
		void on_qpbYesClicked();
		void on_qpbNoClicked();
};

#endif // MUMBLE_MUMBLE_PLUGININSTALLER_H_
