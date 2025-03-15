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

#include <QDebug>
#include <QLoggingCategory>

#ifdef QT_DEBUG
	#define LOGMINLEVEL  QtDebugMsg
#else
	#define LOGMINLEVEL  QtInfoMsg
#endif

static Q_LOGGING_CATEGORY(lcPlugin, "Plugin", LOGMINLEVEL)
static Q_LOGGING_CATEGORY(lcDevices, "Devices", LOGMINLEVEL)
// static Q_LOGGING_CATEGORY(lcDeviceEvents, "Devices.events", LOGMINLEVEL)

#define LOG_SET_W(Text, Width)    qSetFieldWidth(Width) << Text << qSetFieldWidth(0)
#define LOG_HEX(Text, Width)      Qt::hex << Qt::uppercasedigits << qSetPadChar('0') << LOG_SET_W(Text, Width) << Qt::dec << qSetPadChar(' ')
#define LOG_HEX4(Text)            LOG_HEX(Text, 4)
#define LOG_HEX8(Text)            LOG_HEX(Text, 8)

#define DBG_SEP ", "

#define Q_DC(Class)  Q_D(const Class)
