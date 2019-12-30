#include "PluginInstaller.h"

#include <quazipfile.h>
#include <JlCompress.h>

#include <QtCore/QString>
#include <QtCore/QException>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QDir>

#include <QtGui/QIcon>

#include <exception>
#include <string>

#include "PluginManager.h" // This holds the plugin-path-macro

PluginInstallException::PluginInstallException(const QString& msg) : msg(msg) {
}

QString PluginInstallException::getMessage() const {
	return this->msg;
}

const QString PluginInstaller::pluginFileExtension = QLatin1String("mumble_plugin");

bool PluginInstaller::canBePluginFile(const QFileInfo& fileInfo) noexcept {
	if (!fileInfo.isFile()) {
		// A plugin file has to be a file (obviously)
		return false;
	}

	if (fileInfo.suffix().compare(PluginInstaller::pluginFileExtension, Qt::CaseInsensitive) == 0
			|| fileInfo.suffix().compare(QLatin1String("zip"), Qt::CaseInsensitive) == 0) {
		// A plugin file has either the extension given in PluginInstaller::pluginFileExtension or zip
		return true;
	}

	// We might also accept a shared library directly
	return QLibrary::isLibrary(fileInfo.fileName());
}

PluginInstaller::PluginInstaller(const QFileInfo& fileInfo, QWidget *p) : QDialog(p), pluginArchive(fileInfo), plugin(nullptr), pluginSource(),
	pluginDestination(), copyPlugin(false) {
	setupUi(this);

	this->setWindowIcon(QIcon(QLatin1String("skin:mumble.svg")));

	QObject::connect(this->qpbYes, &QPushButton::clicked, this, &PluginInstaller::on_qpbYesClicked);
	QObject::connect(this->qpbNo, &QPushButton::clicked, this, &PluginInstaller::on_qpbNoClicked);

	this->init();
}

PluginInstaller::~PluginInstaller() {
	if (this->plugin) {
		delete this->plugin;
	}
}

void PluginInstaller::init() {
	if (!PluginInstaller::canBePluginFile(this->pluginArchive)) {
		throw PluginInstallException(QObject::tr("The file \"%1\" is not a valid plugin file!").arg(this->pluginArchive.fileName()));
	}

	if (QLibrary::isLibrary(this->pluginArchive.fileName())) {
		// For a library the fileInfo provided is already the actual plugin library
		this->pluginSource = this->pluginArchive;

		this->copyPlugin = true;
	} else {
		// We have been provided with a zip-file
		QuaZip pluginZip(this->pluginArchive.fileName());
		pluginZip.setUtf8Enabled(true);

		if (!pluginZip.open(QuaZip::Mode::mdUnzip)) {
			throw PluginInstallException(QObject::tr("Unable to open plugin archive \"%1\"!").arg(this->pluginArchive.fileName()));
		}

		QStringList fileNames = pluginZip.getFileNameList();

		if (fileNames.isEmpty()) {
			throw PluginInstallException(QObject::tr("Plugin archive \"%1\" does not contain any entries!").arg(this->pluginArchive.fileName()));
		}

		QString pluginName;
		foreach(QString currentFileName, fileNames) {
			if (QLibrary::isLibrary(currentFileName)) {
				if (!pluginName.isEmpty()) {
					// There seem to be multiple plugins in here. That's not allowed
					throw PluginInstallException(QObject::tr("Found more than one plugin library for the current OS in \"%1\" (\"%2\" and \"%3\")!").arg(
								this->pluginArchive.fileName()).arg(pluginName).arg(currentFileName));
				}

				pluginName = currentFileName;
			}
		}

		if (pluginName.isEmpty()) {
			throw PluginInstallException(QObject::tr("Unable to find a plugin for this OS in \"%1\"").arg(this->pluginArchive.fileName()));
		}

		// Unpack the plugin library
		QString tmpPluginPath = QDir::temp().filePath(pluginName);
		if (JlCompress::extractFile(pluginZip.getZipName(), pluginName, tmpPluginPath).isEmpty()) {
			throw PluginInstallException(QObject::tr("Unable to extract plugin to \"%1\"").arg(tmpPluginPath));
		}

		this->pluginSource = QFileInfo(tmpPluginPath);
	}

	QString pluginFileName = this->pluginSource.fileName();

	// Try to load the plugin up to see if it is actually valid
	try {
		this->plugin = Plugin::createNew<Plugin>(this->pluginSource.absoluteFilePath());
	} catch(const PluginError&) {
		throw PluginInstallException(QObject::tr("Unable to load plugin \"%1\" - check the plugin interface!").arg(pluginFileName));
	}

	this->pluginDestination = QFileInfo(QString::fromLatin1("%1/%2").arg(static_cast<QString>(PLUGIN_USER_PATH).isEmpty() ?
				QLatin1String(".") : static_cast<QString>(PLUGIN_USER_PATH)).arg(pluginFileName));


	// Now that we located the plugin, it is time to fill in its details in the UI
	this->qlName->setText(this->plugin->getName());

	version_t pluginVersion = this->plugin->getVersion();
	version_t usedAPIVersion = this->plugin->getAPIVersion();
	this->qlVersion->setText(QString::fromLatin1("%1 (API %2)").arg(pluginVersion == VERSION_UNKNOWN ?
				QLatin1String("Unknown") : static_cast<QString>(pluginVersion)).arg(usedAPIVersion));

	this->qlAuthor->setText(this->plugin->getAuthor());

	this->qlDescription->setText(this->plugin->getDescription());
}

void PluginInstaller::install() const {
	if (!this->plugin) {
		// This function shouldn't even be called, if the pluin obect has not been created...
		throw PluginInstallException(QLatin1String("[INTERNAL ERROR]: Trying to install an invalid plugin"));
	}

	if (this->pluginSource == this->pluginDestination) {
		// Apparently the plugin is already installed
		return;
	}

	if (this->pluginDestination.exists()) {
		// Delete old version first
		if (!QFile(this->pluginDestination.absoluteFilePath()).remove()) {
			throw PluginInstallException(QObject::tr("Unable to delete old plugin at \"%1\"").arg(this->pluginDestination.absoluteFilePath()));
		}
	}

	if (this->copyPlugin) {
		if (!QFile(this->pluginSource.absoluteFilePath()).copy(this->pluginDestination.absoluteFilePath())) {
			throw PluginInstallException(QObject::tr("Unable to copy plugin library from \"%1\" to \"%2\"").arg(this->pluginSource.absoluteFilePath()).arg(
						this->pluginDestination.absoluteFilePath()));
		}
	} else {
		// Move the plugin into the respective dir
		if (!QFile(this->pluginSource.absoluteFilePath()).rename(this->pluginDestination.absoluteFilePath())) {
			throw PluginInstallException(QObject::tr("Unable to move plugin library to \"%1\"").arg(this->pluginDestination.absoluteFilePath()));
		}
	}
}

void PluginInstaller::on_qpbYesClicked() {
	this->install();

	this->close();
}

void PluginInstaller::on_qpbNoClicked() {
	this->close();
}
