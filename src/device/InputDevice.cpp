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

#include "InputDevice.h"

using namespace Devices;

class InputDevicePrivate
{
	Q_DECLARE_PUBLIC(InputDevice)
	InputDevicePrivate(InputDevice *q) : q_ptr(q) { }

	DeviceState state;
	DeviceDescriptor descriptor;
	QString instanceName;
	InputDevice * const q_ptr;
};



InputDevice::InputDevice(QObject *parent) :
  InputDevice(*new InputDevicePrivate(this), parent)
{
}

InputDevice::InputDevice(InputDevicePrivate &dd, QObject *parent) :
  QObject{parent},
  d_ptr(&dd)
{

}

InputDevice::InputDevice(const DeviceDescriptor &dd, QObject *parent) :
  InputDevice(parent)
{
	Q_D(InputDevice);
	d->descriptor = dd;
}

InputDevice::InputDevice(DeviceDescriptor &&dd, QObject *parent) :
  InputDevice(parent)
{
	Q_D(InputDevice);
	d->descriptor = std::move(dd);
}

InputDevice::~InputDevice()
{
	delete d_ptr;
}

void InputDevice::setUid(const QByteArray &id)
{
	if (id == uid())
		return;

	descriptor().uid = id;
	Q_EMIT uidChanged(id);
}

QString InputDevice::name() const {
	Q_DC(InputDevice);
	if (d->instanceName.isEmpty())
		return descriptor().name;
	return d->instanceName;
}

void InputDevice::setName(const QString &name)
{
	if (name == this->name())
		return;

	Q_D(InputDevice);
	if (name == descriptor().name)
		d->instanceName.clear();
	else
		d->instanceName = name;
	// descriptor().name = name;
	Q_EMIT nameChanged(name);
}

Devices::DeviceState InputDevice::state() const {
	return d_ptr->state;
}

void InputDevice::setState(Devices::DeviceState newState)
{
	if (newState == state())
		return;
	Q_D(InputDevice);
	const DeviceState prevState = d->state;
	d->state = newState;
	Q_EMIT stateChanged(newState, prevState);
}

const DeviceDescriptor &InputDevice::descriptor() const {
	return d_ptr->descriptor;
}

DeviceDescriptor &InputDevice::descriptor() {
	Q_D(InputDevice);
	return d->descriptor;
}

