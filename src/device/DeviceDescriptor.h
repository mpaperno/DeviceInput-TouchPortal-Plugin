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

#include "logging.h"
#include "devices.h"

struct DeviceHwData
{
	uint16_t VID;
	uint16_t PID;
	uint16_t version;
	QString vendor {};
	QString product {};
	QString serial {};
	QByteArray path {};

	friend QDebug operator<<(QDebug dbg, const DeviceHwData &hw) {
		QDebugStateSaver saver(dbg);
		dbg.nospace() << "HwData{"
			<< LOG_HEX(hw.VID, 4) << '/'
			<< LOG_HEX(hw.PID, 4) << " @"
			<< LOG_HEX(hw.version, 4) << DBG_SEP
		  << hw.vendor << DBG_SEP
		  << hw.product << DBG_SEP
		  << hw.serial << DBG_SEP
			<< qPrintable(hw.path)
		  << '}';
		return dbg;
	}
};

struct DeviceDescriptor
{
	Devices::DeviceAPI api { Devices::DA_Unknown };
	Devices::DeviceTypes type { Devices::DT_Unknown };
	QByteArray uid {};
	// QUuid guid {};
	uint32_t apiId { 0 };  // zero is invalid
	uint8_t instance { 0 };  // for multiple devices of same type
	QString name {};
	DeviceHwData hwData {};

	friend QDebug operator<<(QDebug dbg, const DeviceDescriptor &dd) {
		QDebugStateSaver saver(dbg);
		dbg.nospace() << "Device {"
			<< dd.type << DBG_SEP
			<< dd.api << DBG_SEP
		  << dd.apiId << DBG_SEP
		  << dd.name << DBG_SEP
		  << dd.instance << DBG_SEP
		  << dd.uid << DBG_SEP
		  // << dd.guid << DBG_SEP
			<< dd.hwData
			<< '}';
		return dbg;
	}
};

Q_DECLARE_METATYPE(DeviceDescriptor)
