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
#include <QCoreApplication>

#include "events.h"

namespace Devices {
// class DeviceEvent;
struct DisplayInfo;
}
class InputDevice;
struct DeviceDescriptor;
class DeviceManagerPrivate;

class DeviceManager : public QObject
{
		Q_OBJECT
	public:
		enum DeviceSortOrder {
			Unordered, DiscoveryOrder, NameOrder
		};

		~DeviceManager();

		static DeviceManager *instance();

		InputDevice *device(const QByteArray &uid) const;
		InputDevice *deviceByName(const QString &name, Qt::MatchFlags matchFlags = Qt::MatchExactly | Qt::MatchCaseSensitive) const;
		QList<InputDevice *> devicesByName(const QString &name, Qt::MatchFlags matchFlags = Qt::MatchExactly | Qt::MatchCaseSensitive, qsizetype maxHits = 0) const;
		QList<InputDevice*> devices(
		    Devices::DeviceState minState = Devices::DeviceState::DS_Seen,
		    DeviceSortOrder order = DeviceSortOrder::Unordered,
		    Devices::DeviceTypes type = Devices::DeviceType::DT_Unknown,
		    qsizetype maxHits = 0) const;
		QStringList deviceNames(
		    Devices::DeviceState minState = Devices::DeviceState::DS_Seen,
		    DeviceSortOrder order = DeviceSortOrder::Unordered,
		    Devices::DeviceTypes type = Devices::DeviceType::DT_Unknown) const;

	public Q_SLOTS:
		void init();
		void deinit();
		void setControllerUpdateInterval(int ms);
		void resetControllerUpdateInterval();
		void updateDevices();
		void startDeviceReport(const QByteArray &uid) const;
		void stopDeviceReport(const QByteArray &uid) const;
		void toggleDeviceReport(const QByteArray &uid) const;
		void requestDeviceReport(const QByteArray &uid) const;

	Q_SIGNALS:
		void deviceDiscovered(const QByteArray &uid);
		void deviceConnected(const QByteArray &uid);
		void deviceRemoved(const QByteArray &uid);
		void deviceReportStarted(const QByteArray &uid);
		void deviceReportStopped(const QByteArray &uid);
		void deviceEvent(const Devices::DeviceEvent &ev);
		void deviceEventPtr(const Devices::DeviceEvent *ev);
		void deviceNameChanged(InputDevice *dev, const QString &name);
		void deviceStateChanged(InputDevice *dev, Devices::DeviceState newState, Devices::DeviceState previousState = Devices::DeviceState::DS_Unknown);
		void displayDetected(const Devices::DisplayInfo &displayInfo);
		void displayRemoved(short id);

	protected Q_SLOTS:
		void onPlatformDeviceDiscovered(const DeviceDescriptor &dd);
		void onPlatformDeviceRemoved(const QByteArray &uid);
		void onPlatformDeviceEvent(Devices::DeviceEvent *ev);
		void onPlatformDeviceReportToggled(const QByteArray &uid, bool started);
		void onDevNameChanged(const QString &name);
		void onDevStateChanged(Devices::DeviceState newState, Devices::DeviceState previousState);

	protected:
		explicit DeviceManager(QObject *parent = nullptr);

	private:
		DeviceManagerPrivate* const d_ptr;
		Q_DECLARE_PRIVATE(DeviceManager)
		Q_DISABLE_COPY(DeviceManager)
};
