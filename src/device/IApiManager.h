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

// #include "events.h"
// #include "devices.h"

namespace Devices {
class DeviceEvent;
struct DisplayInfo;
}
struct DeviceDescriptor;

class IApiManager : public QObject
{
		Q_OBJECT
	public:
		using QObject::QObject;
		virtual ~IApiManager() = default;

		virtual bool init() = 0;
		virtual void deinit() = 0;

		virtual inline QString getLastError() const { return m_lastError; }

	public Q_SLOTS:
		// void setScanInterval(uint ms);
		virtual void scanDevices() = 0;
		virtual void connectDevice(const QByteArray &uid) = 0;
		virtual void disconnectDevice(const QByteArray &uid) = 0;
		virtual void sendDeviceReport(const QByteArray &uid) = 0;

	Q_SIGNALS:
		void deviceEvent(Devices::DeviceEvent *ev);
		void deviceDiscovered(const DeviceDescriptor &dd);
		void deviceRemoved(const QByteArray &uid);
		void deviceReportToggled(const QByteArray &uid, bool started);
		void displayDetected(const Devices::DisplayInfo &displayInfo);
		void displayRemoved(short id);

	protected:
		void setLastError(const QString &msg) {
			m_lastError = msg;
		}

		void clearLastError() {
			setLastError(QString());
		}

		QString m_lastError;
};
