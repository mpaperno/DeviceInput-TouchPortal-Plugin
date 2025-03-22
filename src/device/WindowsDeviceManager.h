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

#include "IApiManager.h"

namespace Devices {
class DeviceEvent;
}
class WindowsHookWorker;
class QThread;

class WindowsDeviceManager : public IApiManager
{
		Q_OBJECT
	public:
		explicit WindowsDeviceManager(QObject *parent = nullptr);
		~WindowsDeviceManager();

		bool init() override;
		void deinit() override;

	public Q_SLOTS:
		void scanDevices() override;
		void connectDevice(const QByteArray &uid) override;
		void disconnectDevice(const QByteArray &uid) override;
		void sendDeviceReport(const QByteArray &uid) override;

		void keyEventHandler(uint32_t vkCode, uint32_t scanCode, uint32_t flags);
		void mouseEventHandler(uint32_t time, long ptX, long ptY, uint8_t button = 0, bool pressOrWheelH = false, int16_t wheel = 0);

	private Q_SLOTS:
		void createWorker();
		void destroyWorker();
		void onDeviceHooked(const QByteArray &, bool started);
		// void discoverScreens();

	Q_SIGNALS:
		void enableDeviceHook(const QByteArray &, bool);
		void enableKeyboardHook(bool);
		void enableMouseHook(bool);

	private:
		WindowsHookWorker *m_hookWorker = nullptr;
		QThread *m_workerThread = nullptr;
};
