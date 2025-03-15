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
#include "IApiManager.h"

namespace Devices {
class DeviceEvent;
}
struct DeviceDescriptor;
class SDLManagerPrivate;

class SDLManager : public IApiManager
{
		Q_OBJECT
	public:
		explicit SDLManager(QObject *parent = nullptr);
		~SDLManager();

		bool init() override;
		void deinit() override;

		QString getLastError() const override;

		int activeScanInterval() const;
		int defaultActiveScanInterval() const;
		int idleScanInterval() const;
		int defaultIdleScanInterval() const;

	public Q_SLOTS:
		void setActiveScanInterval(int ms);
		void setIdleScanInterval(int ms);

		void scanDevices() override;
		void connectDevice(const QByteArray &uid) override;
		void disconnectDevice(const QByteArray &uid) override;
		void sendDeviceReport(const QByteArray &uid) override;

	private:
		SDLManagerPrivate* const d_ptr;
		Q_DECLARE_PRIVATE(SDLManager)
};

// }
