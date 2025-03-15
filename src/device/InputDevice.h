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

#include "devices.h"
#include "DeviceDescriptor.h"

// namespace Devices {

class InputDevicePrivate;

class InputDevice : public QObject
{
		Q_OBJECT
	public:
		explicit InputDevice(QObject *parent = nullptr);
		explicit InputDevice(const DeviceDescriptor &dd, QObject *parent = nullptr);
		explicit InputDevice(DeviceDescriptor &&dd, QObject *parent = nullptr);

		~InputDevice();

		Devices::DeviceTypes type() const { return descriptor().type; }
		Devices::DeviceAPI api() const { return descriptor().api; }


		QByteArray uid() const { return descriptor().uid; }
		void setUid(const QByteArray &id);

		QString name() const;
		void setName(const QString &name);
		void resetName() { setName(descriptorName()); }
		/// The original name from system, before any changes for instance numbers/etc.
		QString descriptorName() const { return descriptor().name; }

		uint8_t instance() const { return descriptor().instance; }
		void setInstance(uint8_t number) { descriptor().instance = number; }

		Devices::DeviceState state() const;
		void setState(Devices::DeviceState newState);

		virtual DeviceDescriptor const &descriptor() const;

	Q_SIGNALS:
		void uidChanged(const QByteArray &name);
		void nameChanged(const QString &name);
		void stateChanged(Devices::DeviceState newState, Devices::DeviceState previousState);

	protected:
		InputDevice(InputDevicePrivate &dd, QObject *parent = nullptr);

		virtual DeviceDescriptor &descriptor();

		InputDevicePrivate* const d_ptr;

	private:
		Q_DECLARE_PRIVATE(InputDevice)
		Q_DISABLE_COPY(InputDevice)

};

// }
