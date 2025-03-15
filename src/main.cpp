/*
Device Input Plugin for Touch Portal
Copyright Maxim Paperno; all rights reserved.

This file may be used under the terms of the GNU
General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License is available at <http://www.gnu.org/licenses/>.

This project may also use 3rd-party Open Source software under the terms
of their respective licenses. The copyright notice above does not apply
to any 3rd-party components used within.
*/

#include <QtGlobal>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QSettings>

// #ifdef Q_OS_WIN
// #include <qt_windows.h>
// #endif
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <csignal>

#include "logging.h"
// #include "ExceptionHandler.h"
#include "Logger.h"
#include "Plugin.h"
#include "RunGuard.h"
// #include "TPClientQt.h"
#include "version.h"


// configure logging categories externally:
// Set env. var QT_LOGGING_RULES to override, eg:
//   QT_LOGGING_RULES="*.debug=false;DSE.debug=true"
// or set QT_LOGGING_CONF to point to a config file, see QLoggingCategory docs.
// Override message pattern by setting QT_MESSAGE_PATTERN env. var.

#ifdef QT_DEBUG
	#define MESSAGE_LOG_PATTERN  \
		"[%{time process}] " \
		"[%{if-debug}DBG%{endif}%{if-info}INF%{endif}%{if-warning}WRN%{endif}%{if-critical}ERR%{endif}%{if-fatal}CRT%{endif}] " \
		"%{if-category}|%{category}| %{endif}%{function}() @%{line} - %{message}"
#else
	#define MESSAGE_LOG_PATTERN  \
		"[%{time MM-dd HH:mm:ss.zzz}] " \
		"[%{if-debug}DBG%{endif}%{if-info}INF%{endif}%{if-warning}WRN%{endif}%{if-critical}ERR%{endif}%{if-fatal}CRT%{endif}] " \
		"%{if-category}|%{category}| %{endif}- %{message}"
#endif

#define APP_DBG_HANDLER_NO_REPLACE_QML  1

#define OPT_LOGSTDO   QStringLiteral("s")  // enable console/stdout @ level
#define OPT_LOGMAIN   QStringLiteral("f")  // enable primary log file @ level
#define OPT_LOGKEEP   QStringLiteral("k")  // keep log days
#define OPT_LOGPATH   QStringLiteral("p")  // log path
#define OPT_LOGSROT   QStringLiteral("r")  // rotate logs now
#define OPT_XITERLY   QStringLiteral("x")  // exit w/out starting
#define OPT_TPHOSTP   QStringLiteral("t")  // TP host:port
#define OPT_PLUGNID   QStringLiteral("i")  // plugin ID

void sigHandler(int s)
{
	std::signal(s, SIG_DFL);
	qApp->quit();
}

QString logfilesPath;

int main(int argc, char *argv[])
{
	// set up message logging pattern
	qSetMessagePattern(MESSAGE_LOG_PATTERN);

	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(PLUGIN_SYSTEM_NAME);
	QCoreApplication::setOrganizationName(PLUGIN_ORG_NAME);
	// QCoreApplication::setOrganizationDomain(PLUGIN_DOMAIN);
	QCoreApplication::setApplicationVersion(APP_VERSION_STR);
	QSettings::setDefaultFormat(QSettings::IniFormat);

	// appStartupArgs = QCoreApplication::arguments();

	// Figure out default log path; should go at same level as bin direcotry or app bundle.
	logfilesPath = QCoreApplication::applicationDirPath() + '/';
	if (logfilesPath.endsWith(QLatin1String("bin/")))
		logfilesPath += QLatin1String("../");
	else if (logfilesPath.endsWith(QLatin1String(".app/Contents/MacOS/")))
		logfilesPath += QLatin1String("../../../");
	logfilesPath = QDir::cleanPath(logfilesPath + QLatin1String("logs"));

	Logger::instance()->installAppMessageHandler();
	// ExceptionHandler::initExceptionHandler();
	// ExceptionHandler::initSigHandler();

	// Empty TP host and port 0 will leave TPClient's default as-is.
	QString tpHost;
	uint16_t tpPort = 0;

	// Set default logging levels for file and stderr.
	qint8 keep = 3,      // # of rotations to keep
	    fileLevel = 1;
#ifdef QT_DEBUG
	qint8 stdoutLevel = 0;  // enable @ debug level
#else
	qint8 stdoutLevel = 5;  // disable for release builds
#endif

	QCommandLineParser clp;
	clp.setApplicationDescription("\n" PLUGIN_NAME "\n\n" "Logging levels for options, most to least verbose: 0 = Debug; 1 = Info; 2 = Warning; 3 = Error; 4 = Fatal; 5 = disable logging.");
	clp.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);
	clp.addOptions({
		{ {OPT_LOGMAIN, QStringLiteral("file")},    qApp->translate("main", "Enable logging to primary plugin log file at given verbosity level. Default is %1").arg(fileLevel), QStringLiteral("level"), QString::number(fileLevel) },
		{ {OPT_LOGSTDO, QStringLiteral("stdout")},  qApp->translate("main", "Enable logging output to the system console/stdout at given verbosity level. Default is %1").arg(stdoutLevel), QStringLiteral("level"), QString::number(stdoutLevel) },
		{ {OPT_LOGPATH, QStringLiteral("path")},    qApp->translate("main", "Path for log files. Default is '%1'").arg(logfilesPath), QStringLiteral("path") },
		{ {OPT_LOGKEEP, QStringLiteral("keep")},    qApp->translate("main", "Keep this number of previous logs (logs are rotated daily, default is to keep %1 days plus the current day).").arg(keep), QStringLiteral("days") },
		{ {OPT_LOGSROT, QStringLiteral("rotate")},  qApp->translate("main", "Rotate log file(s) on startup (starts with empty logs). Only enabled log(s) (with -f or -j) are rotated.") },
		{ {OPT_XITERLY, QStringLiteral("exit")},    qApp->translate("main", "Exit w/out starting. For example after rotating logs.") },
		{ {OPT_TPHOSTP, QStringLiteral("tphost")},  qApp->translate("main", "Touch Portal host address and optional port number in the format of 'host_name_or_address[:port_number]'. Default is '127.0.0.1:12136'."), QStringLiteral("host[:port]") },
		{ {OPT_PLUGNID, QStringLiteral("pluginid")},qApp->translate("main", "Use a custom Touch Portal Plugin ID for this instance (only use with custom entry.tp)."), QStringLiteral("ID") },
	});
	clp.addHelpOption();
	clp.addVersionOption();
	clp.process(a);

	QString pluginId {};  // leave empty for default
	if (clp.isSet(OPT_PLUGNID) && clp.value(OPT_PLUGNID) != PLUGIN_ID) {
		pluginId = clp.value(OPT_PLUGNID);
		QCoreApplication::setOrganizationName(pluginId);
		QCoreApplication::setOrganizationDomain(pluginId);
	}

	// Prevent multiple instances.
	RunGuard guard( pluginId.isEmpty() ? PLUGIN_ID : pluginId );
	if (!guard.tryToRun()) {
		std::cout << "Another instance of this application with ID " << guard.keyString().toStdString() << " is already running. Quitting now." << std::endl;
		return 0;
	}


	if (clp.isSet(OPT_LOGPATH))
		logfilesPath = clp.value(OPT_LOGPATH);

	bool ok;
	if (clp.isSet(OPT_LOGKEEP)) {
		quint8 k = clp.value(OPT_LOGKEEP).toUInt(&ok);
		if (ok)
			keep = k;
		else
			clp.showHelp(1);
	}

	if (clp.isSet(OPT_LOGMAIN)) {
		quint8 k = clp.value(OPT_LOGMAIN).toUInt(&ok);
		if (ok)
			fileLevel = k;
		else
			clp.showHelp(1);
	}

	if (clp.isSet(OPT_LOGSTDO)) {
		quint8 k = clp.value(OPT_LOGSTDO).toUInt(&ok);
		if (ok)
			stdoutLevel = k;
		else
			clp.showHelp(1);
	}

	if (clp.isSet(OPT_TPHOSTP)) {
		const auto optlist = clp.value(OPT_TPHOSTP).split(':');
		tpHost = optlist.first();
		if (optlist.length() > 1) {
			quint8 k = optlist.at(1).toUInt(&ok);
			if (ok)
				tpPort = k;
			else
				clp.showHelp(1);
		}
	}

	QString logFilterRules;

	quint8 effectiveLevel = fileLevel > -1 ? std::min(stdoutLevel, fileLevel) : stdoutLevel;
	const quint8 catLevel = std::max(Logger::levelForCategory(lcPlugin()), Logger::levelForCategory(lcDevices()));
	//std::cerr << (int)effectiveLevel << ' ' << (int)Logger::levelForCategory(lcPlugin()) << std::endl;
	if (effectiveLevel < 5 && effectiveLevel > catLevel) {
		while (effectiveLevel > 0 ) {
			const QLatin1String lvlName = QLatin1String(Logger::logruleNameForLevel(effectiveLevel-1));
			logFilterRules += QLatin1String(lcPlugin().categoryName()) + '.' + lvlName + " = false\n";
			logFilterRules += QLatin1String(lcLog().categoryName()) + '.' + lvlName + " = false\n";
			logFilterRules += QLatin1String(lcDevices().categoryName()) + '.' + lvlName + " = false\n";
			logFilterRules += QLatin1String("TPClientQt.") + lvlName + " = false\n";
			--effectiveLevel;
		}
	}
	else {
		while (effectiveLevel < catLevel) {
			const QLatin1String lvlName = QLatin1String(Logger::logruleNameForLevel(effectiveLevel));
			logFilterRules += QLatin1String(lcPlugin().categoryName()) + '.' + lvlName + " = true\n";
			logFilterRules += QLatin1String(lcLog().categoryName()) + '.' + lvlName + " = true\n";
			logFilterRules += QLatin1String(lcDevices().categoryName()) + '.' + lvlName + " = true\n";
			logFilterRules += QLatin1String("TPClientQt.") + lvlName + " = true\n";
			++effectiveLevel;
		}
	}

	if (!logFilterRules.isEmpty())
		QLoggingCategory::setFilterRules(logFilterRules);
	// std::cout << logFilterRules.toStdString() << std::endl;

	Logger::instance()->setAppDebugOutputLevel(stdoutLevel);
	if (fileLevel > -1 && fileLevel < 5)
		Logger::instance()->addFileDevice(logfilesPath + "/plugin.log", fileLevel, {}, true, keep);

	if (clp.isSet(OPT_LOGSROT))
		Logger::instance()->rotateLogs();

	if (clp.isSet(OPT_XITERLY)) {
		QTimer::singleShot(1000, qApp, [&]() { qApp->quit(); });
		return a.exec();
	}

	std::signal(SIGTERM, sigHandler);
  std::signal(SIGABRT, sigHandler);
  std::signal(SIGINT, sigHandler);
#ifdef Q_OS_WIN
  std::signal(SIGBREAK, sigHandler);
	// _setmode(_fileno(stdout), _O_U16TEXT);
#endif

	Plugin p(tpHost, tpPort, pluginId.toUtf8());
	return a.exec();
}
