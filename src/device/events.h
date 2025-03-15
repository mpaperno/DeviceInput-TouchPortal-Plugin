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

#include "devices.h"
#include "logging.h"

class InputDevice;

namespace Devices {

struct DeviceEvent
{
		DeviceEvent() { }

		DeviceEvent(const DeviceEvent &other) = default;
		DeviceEvent(DeviceEvent &&other) = default;
		// DeviceEvent(const DeviceEvent &other) :
		// 	type(other.type), timestamp(other.timestamp), deviceType(other.deviceType),
		// 	device(other.device), deviceUid(other.deviceUid), index{other.index}
		// {}
		virtual ~DeviceEvent() { };

		virtual DeviceEvent *clone() const { return new DeviceEvent(*this); };

		const EventType type { EventType::Event_Generic };
		uint64_t timestamp { 0 };
		DeviceTypes deviceType { DeviceType::DT_Unknown };
		InputDevice *device = nullptr;
		QByteArray deviceUid {};
		uint index { 0 };  //< index of originating control

		friend QDebug operator <<(QDebug dbg, const DeviceEvent &ev) {
			QDebugStateSaver saver(dbg);
			return dbg.nospace() << '{'
				<< ev.timestamp << DBG_SEP << ev.type << DBG_SEP << Devices::deviceTypeName(ev.deviceType) << DBG_SEP
				<< ev.deviceUid << DBG_SEP << LOG_HEX4(ev.index)
				<< " | ";
		}

	protected:
		DeviceEvent(EventType type, uint index = 0, uint64_t timestamp = 0UL,
		            DeviceTypes deviceType = DeviceType::DT_Unknown, const QByteArray &deviceUid = QByteArray()) :
		  type{type}, timestamp{timestamp}, deviceType{deviceType}, deviceUid{deviceUid}, index{index} {}
};

struct DeviceAxisEvent : public DeviceEvent
{
		// using DeviceEvent::DeviceEvent;
		DeviceAxisEvent(uint64_t timestamp = 0UL, uint8_t index = 0, float value = 0.0f) :
		  DeviceEvent(EventType::Event_Axis, index, timestamp),
			value{value} {}

		DeviceAxisEvent(const DeviceAxisEvent &other) = default;
		DeviceAxisEvent(DeviceAxisEvent &&other) = default;
		// DeviceAxisEvent(const DeviceAxisEvent &other) : DeviceEvent(other),
		// 	value{other.value} {}

		DeviceAxisEvent *clone() const override { return new DeviceAxisEvent(*this); }

		// uint8_t index;
		float value;

		friend QDebug operator <<(QDebug dbg, const DeviceAxisEvent &ev) {
			QDebugStateSaver saver(dbg);
			return dbg.nospace() << (DeviceEvent)ev << ev.value << '}';
		}
};

struct DeviceHatEvent : public DeviceEvent
{
		// using DeviceEvent::DeviceEvent;
		DeviceHatEvent(uint64_t timestamp = 0UL, uint8_t index = 0, int value = 0) :
		  DeviceEvent(EventType::Event_Hat, index, timestamp),
			value{value} {}

		DeviceHatEvent(const DeviceHatEvent &other) = default;
		DeviceHatEvent(DeviceHatEvent &&other) = default;
		// DeviceHatEvent(const DeviceHatEvent &other) : DeviceEvent(other),
		// 	value{other.value} {}

		DeviceHatEvent *clone() const override { return new DeviceHatEvent(*this); }

		// uint8_t index;
		int value;

		friend QDebug operator <<(QDebug dbg, const DeviceHatEvent &ev) {
			QDebugStateSaver saver(dbg);
			return dbg.nospace() << (DeviceEvent)ev << ev.value << '}';
		}
};

struct DeviceKeyEvent : public DeviceEvent
{
		// using DeviceEvent::DeviceEvent;
		DeviceKeyEvent(uint64_t timestamp = 0UL, uint scancode = 0,
		               uint modifiers = 0, uint sdlKey = 0, uint vk = 0, uint nSC = 0, bool down = false,
		               bool rep = false, const QString &name = QString(), const QString &text = QString(),
		               const QByteArray &deviceUid = DI_SYSTEM_KEYBOARD_UID) :
		  DeviceEvent(EventType::Event_Key, scancode, timestamp, DeviceType::DT_Keyboard, deviceUid),
			modifiers(modifiers), sdlKey(sdlKey),
		  nativeKey(vk), nativeScanCode(nSC), down(down), repeat(rep),
		  name(name), text(text) {}

		DeviceKeyEvent(const DeviceKeyEvent &other) = default;
		DeviceKeyEvent(DeviceKeyEvent &&other) = default;

		// DeviceKeyEvent(const DeviceKeyEvent &other) : DeviceEvent(other),
		//   modifiers(other.modifiers), sdlKey(other.sdlKey),
		// 	nativeKey(other.nativeKey), nativeScanCode(other.nativeScanCode), down(other.down), repeat(other.repeat),
		//   name(other.name), text(other.text) {}

		DeviceKeyEvent *clone() const override { return new DeviceKeyEvent(*this); }

		inline uint key() const { return index; }
		inline uint scancode() const { return index; }

		uint modifiers;
		uint sdlKey;
		uint nativeKey;
		uint nativeScanCode;
		bool down;
		bool repeat;
		QString name;
		QString text;

		friend QDebug operator <<(QDebug dbg, const DeviceKeyEvent &ev) {
			QDebugStateSaver saver(dbg);
			return dbg.nospace() << (DeviceEvent)ev
				<< LOG_HEX4(ev.sdlKey) << DBG_SEP << LOG_HEX4(ev.nativeKey) << DBG_SEP << LOG_HEX4(ev.nativeScanCode) << DBG_SEP
			  << LOG_HEX4(ev.modifiers) << DBG_SEP << ev.down << DBG_SEP << ev.repeat << DBG_SEP << ev.name << DBG_SEP << ev.text
			  << '}';
		}
};

struct DevicePositionEvent : public DeviceEvent
{
		// DevicePositionEvent(const DevicePositionEvent &other) : DeviceEvent(other),
		// 	x{other.x}, y{other.y} {}

		DevicePositionEvent(const DevicePositionEvent &other) = default;
		DevicePositionEvent(DevicePositionEvent &&other) = default;

		float x;
		float y;

		friend QDebug operator <<(QDebug dbg, const DevicePositionEvent &ev) {
			QDebugStateSaver saver(dbg);
			return dbg.nospace() << (DeviceEvent)ev << ev.x << DBG_SEP << ev.y;
		}

	protected:
		explicit DevicePositionEvent(EventType type, uint index = 0, uint64_t timestamp = 0UL, float x = 0.0f, float y = 0.0f,
		                             DeviceTypes devType = DeviceType::DT_Unknown, const QByteArray &uid = QByteArray()) :
		  DeviceEvent(type, index, timestamp, devType, uid),
			x{x}, y{y} {}
};

struct DeviceScrollEvent : public DevicePositionEvent
{
		DeviceScrollEvent(uint64_t timestamp = 0UL, uint8_t index = 0, float x = 0.0f, float y = 0.0f, float relX = 0.0f, float relY = 0.0f) :
			DevicePositionEvent(EventType::Event_Scroll, index, timestamp, x, y),
		  relX{relX}, relY{relY} {}

		DeviceScrollEvent(uint64_t timestamp, DeviceTypes type, const QByteArray &uid, uint8_t index = 0, float x = 0.0f, float y = 0.0f, float relX = 0.0f, float relY = 0.0f) :
		  DevicePositionEvent(EventType::Event_Scroll, index, timestamp, x, y, type, uid),
		  relX{relX}, relY{relY} {}

		DeviceScrollEvent(const DeviceScrollEvent &other) = default;
		DeviceScrollEvent(DeviceScrollEvent &&other) = default;
		// DeviceScrollEvent(const DeviceScrollEvent &other) : DevicePositionEvent(other),
		//   relX{other.relX}, relY{other.relY} {}

		float relX;
		float relY;

		friend QDebug operator <<(QDebug dbg, const DeviceScrollEvent &ev) {
			QDebugStateSaver saver(dbg);
			return dbg.nospace() << (DevicePositionEvent)ev << ev.relX << DBG_SEP << ev.relY << '}';
		}
};

struct DeviceButtonEvent : public DevicePositionEvent
{
		DeviceButtonEvent(uint64_t timestamp = 0UL, uint8_t index = 0, bool down = false, float x = 0.0f, float y = 0.0f) :
			DevicePositionEvent(EventType::Event_Button, index, timestamp, x, y),
			down{down} {}

		DeviceButtonEvent(uint64_t timestamp, DeviceTypes type, const QByteArray &uid, uint8_t index, bool down = false, float x = 0.0f, float y = 0.0f) :
		  DevicePositionEvent(EventType::Event_Button, index, timestamp, x, y, type, uid),
		  down{down} {}

		DeviceButtonEvent(const DeviceButtonEvent &other) = default;
		DeviceButtonEvent(DeviceButtonEvent &&other) = default;
		// DeviceButtonEvent(const DeviceButtonEvent &other) : DevicePositionEvent(other),
		// 	down{other.down} {}

		DeviceButtonEvent *clone() const override { return new DeviceButtonEvent(*this); }

		bool down;

		friend QDebug operator <<(QDebug dbg, const DeviceButtonEvent &ev) {
			QDebugStateSaver saver(dbg);
			return dbg.nospace() << (DevicePositionEvent)ev << ev.down << '}';
		}
};

struct DeviceMotionEvent : public DevicePositionEvent
{
		DeviceMotionEvent(uint64_t timestamp = 0UL, uint8_t index = 0, float x = 0.0f, float y = 0.0f, float relX = 0.0f, float relY = 0.0f, uint buttons = 0) :
			DevicePositionEvent(EventType::Event_Motion, index, timestamp, x, y),
			relX{relX}, relY{relY}, buttons{buttons} {}

		DeviceMotionEvent(uint64_t timestamp, DeviceTypes type, const QByteArray &uid, uint8_t index = 0, float x = 0.0f, float y = 0.0f, float relX = 0.0f, float relY = 0.0f, uint buttons = 0) :
		  DevicePositionEvent(EventType::Event_Motion, index, timestamp, x, y, type, uid),
		  relX{relX}, relY{relY}, buttons{buttons} {}


		DeviceMotionEvent(const DeviceMotionEvent &other) = default;
		DeviceMotionEvent(DeviceMotionEvent &&other) = default;

		// DeviceMotionEvent(const DeviceMotionEvent &other) : DevicePositionEvent(other),
		// 	relX{other.relX}, relY{other.relY}, buttons{other.buttons} {}

		DeviceMotionEvent *clone() const override { return new DeviceMotionEvent(*this); }

		float relX;
		float relY;
		uint buttons;

		friend QDebug operator <<(QDebug dbg, const DeviceMotionEvent &ev) {
			QDebugStateSaver saver(dbg);
			return dbg.nospace() << (DevicePositionEvent)ev << ev.relX << DBG_SEP << ev.relY << DBG_SEP << ev.buttons << '}';
		}
};

}

Q_DECLARE_METATYPE(Devices::DeviceEvent)
Q_DECLARE_METATYPE(Devices::DevicePositionEvent)
Q_DECLARE_METATYPE(Devices::DeviceAxisEvent)
Q_DECLARE_METATYPE(Devices::DeviceHatEvent)
Q_DECLARE_METATYPE(Devices::DeviceKeyEvent)
Q_DECLARE_METATYPE(Devices::DeviceScrollEvent)
Q_DECLARE_METATYPE(Devices::DeviceButtonEvent)
Q_DECLARE_METATYPE(Devices::DeviceMotionEvent)
