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

#pragma once

#include <QObject>
#include <QTimer>

#include "devices.h"
#include "strings.h"
#include "TPClientQt.h"

QT_BEGIN_NAMESPACE
class QJsonObject;
class QThread;
QT_END_NAMESPACE


namespace Devices {
struct DeviceEvent;
// struct DisplayInfo;
}
class InputDevice;

class Plugin : public QObject
{
		Q_OBJECT
	public:
		explicit Plugin(const QString &tpHost, uint16_t tpPort, const QByteArray &pluginId = QByteArray(), QObject *parent = nullptr);
		~Plugin();

		static Plugin *instance;

	Q_SIGNALS:
		void tpConnect();
		void tpDisconnect();
		void tpStateUpdate(const QByteArray &, const QByteArray &) const;
		// void tpStateUpdateStr(const QByteArray &, QStringView) const;
		void tpStateCreate(const QByteArray &stateId, const QByteArray &parent, const QByteArray &descript, const QByteArray &deflt = QByteArray(), bool = false) const;
		void tpStateRemove(const QByteArray &) const;
		// void tpChoiceUpdate(const QByteArray &, const QByteArrayList &) const;
		void tpChoiceUpdate(const QByteArray &, const QStringList &) const;
		// void tpStateListUpdate(const QByteArray &, const QByteArrayList &) const;
		void tpStateListUpdate(const QByteArray &, const QStringList &) const;
		void tpTriggerEvent(const QByteArray &, const QJsonObject &) const;
		// void tpChoiceUpdateInstance(const QByteArray &, const QByteArray &, const QByteArrayList &) const;
		// void tpChoiceUpdateInstanceStrList(const QByteArray &, const QByteArray &, const QStringList &) const;
		// void tpConnectorUpdate(const QByteArray &, quint8, bool) const;
		// void tpConnectorUpdateShort(const QByteArray &, quint8) const;
		// void tpNotification(const QByteArray &, const QByteArray &, const QByteArray &, const QVariantList &) const;
		void tpSettingUpdate(const QByteArray &, const QByteArray &);

		// void tpMessageEvent(const QJsonObject &msg);
		// void tpNotificationClicked(const QString &, const QString &) const;
		// void tpBroadcast(const QString &, const QVariantMap &) const;

		void loggerRotateLogs() const;

	public Q_SLOTS:
		//void start();
		void exit();
		void quit();

	// protected Q_SLOTS:
		// void timerEvent(QTimerEvent *ev) override;

	private Q_SLOTS:
		void init();

		// void savePluginSettings() const;
		// void loadPluginSettings();
		void loadDefaultDevices();
		void saveDefaultDevices() const;
		// void loadStartupSettings();

		void createStateWithDelay(const QByteArray &stateId, const QByteArray &parent, const QByteArray &name, const QByteArray &dflt = QByteArray(), bool force = false, int delayMs = 2) const;

		void updatePluginState(Strings::ActionTokens state, bool direct = false) const;
		void updateDisplayInfoStates(const Devices::DisplayInfo &si);
		void removeDisplayStates(short nDisplay);
		void setupKeyboardStates(const InputDevice *dev) const;

		void sendInstanceLists() const;
		void sendDeviceInstanceUpdates(const InputDevice *dev) const;
		void sendFirstAssignedDeviceStateUpdate(Devices::DeviceTypes devType) const;
		void sendDefaultAssignedDeviceStateUpdate(Devices::DeviceTypes devType) const;
		void sendFullStatusReport() const;

		void setDefaultDeviceForTypeName(const QString &typeName, const QString &deviceName, bool notify = true, bool save = true);

		void dispatchDeviceEvent(const InputDevice *dev, Strings::ActionTokens event, Strings::StateIdToken stateId = Strings::StateIdToken::SID_ENUM_MAX, Strings::EventIdToken eventId = Strings::EID_ENUM_MAX) const;
		void onDeviceConnected(const QByteArray &uid) const;
		void onDeviceRemoved(const QByteArray &uid) const;
		void onDeviceReportStarted(const InputDevice *dev) const;
		void onDeviceReportStopped(const InputDevice *dev) const;
		void onDeviceNameChanged(const InputDevice *dev, const QString &name) const;
		void onDeviceStateChanged(const InputDevice *dev, Devices::DeviceState newState, Devices::DeviceState previousState = Devices::DeviceState::DS_Unknown) const;
		void onDeviceEvent(const Devices::DeviceEvent &ev);
		void onDeviceEventPtr(const Devices::DeviceEvent *ev);

		void onClientDisconnect();
		void onClientError(QAbstractSocket::SocketError);
		void onTpConnected(const TPClientQt::TPInfo &info, const QJsonObject &settings);
		void onTpMessage(TPClientQt::MessageType type, const QJsonObject &msg);

		void dispatchAction(TPClientQt::MessageType type, const QJsonObject &msg);
		void pluginAction(TPClientQt::MessageType type, int act, const QMap<QString, QString> &dataMap, qint32 connectorValue);
		void handleSettings(const QJsonObject &settings) const;

	private:
		typedef QVarLengthArray<InputDevice *, 1> DeviceListFromActionT;
		DeviceListFromActionT getDeviceFromActionData(const QMap<QString, QString> &dataMap);

		const QByteArray m_pluginId;
		const QByteArray m_pluginStateIdPrefix;
		const QByteArray m_pluginEventIdPrefix;
		const QByteArray m_pluginActIdPrefix;
		TPClientQt *client = nullptr;
		QThread *clientThread = nullptr;
		QTimer m_loadSettingsTmr;
		QTimer m_deviceListTmr;
		// QPair<QByteArray, bool> m_lastDeviceUid;
		QByteArray m_stateIds[Strings::SID_ENUM_MAX];
		QByteArray m_eventIds[Strings::EID_ENUM_MAX];
		QByteArray m_choiceListIds[Strings::CLID_ENUM_MAX];

		QReadWriteLock m_mtxDeviceStates;
		QHash<QByteArray, QHash<QByteArray, QByteArray>> m_deviceStates;
		QHash<Devices::DeviceTypes, QString> m_defaultDevices;
};
