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

#include <QtCore>
#include <cstdint>

#define DI_SYSTEM_KEYBOARD_ID  1
#define DI_SYSTEM_MOUSE_ID     1
#define DI_SYSTEM_KEYBOARD_UID  QByteArrayLiteral("SYSTEM_KEYBOARD")
#define DI_SYSTEM_MOUSE_UID     QByteArrayLiteral("SYSTEM_MOUSE")
#define DI_SYSTEM_SCREEN_UID    QByteArrayLiteral("SYSTEM_SCREEN")

// Init video subsystem in SDL API
#define PLATFORM_SDL_INIT_VIDEO 0

namespace Devices {

Q_NAMESPACE
static inline QString tr(const char *sourceText, const char *disambiguation = nullptr, int n = -1) {
	return QCoreApplication::translate("Devices", sourceText, disambiguation, n);
}

enum DeviceAPI : uint8_t {
	DA_Unknown,
	DA_SDL,
	DA_NATIVE,
};
Q_ENUM_NS(DeviceAPI)

enum DeviceType : uint32_t {
	DT_Unknown = 0,
	DT_Controller = 0x0001,
	DT_Keyboard   = 0x0002,
	DT_Pointer    = 0x0004,
	DT_Sensor     = 0x0008,
	DT_GenericHID = 0x0010,
	// DT_Screen     = 0x0020,

	DT_Joystick   = 0x0100,
	DT_Gamepad    = 0x0200,
	DT_Throttle   = 0x0400,
	DT_Wheel      = 0x0800,
	DT_JoystickType = DT_Controller | DT_Joystick,
	DT_GamepadType  = DT_Controller | DT_Gamepad,
	DT_ThrottleType = DT_Controller | DT_Throttle,
	DT_WheelType    = DT_Controller | DT_Wheel,

	DT_Mouse      = 0x0100,
	DT_Trackpad   = 0x0200,
	DT_Touch      = 0x0400,
	DT_Pen        = 0x0800,
	DT_MouseType    = DT_Pointer | DT_Mouse,
	DT_TrackpadType = DT_Pointer | DT_Trackpad,
	DT_TouchType    = DT_Pointer | DT_Touch,
	DT_PenType      = DT_Pointer | DT_Pen,

	DT_Accel      = 0x0100,
	DT_Gyro       = 0x0200,
	DT_Mag        = 0x0400,
	DT_AccelType    = DT_Sensor | DT_Accel,
	DT_GyroType    = DT_Sensor | DT_Gyro,
	DT_MagType    = DT_Sensor | DT_Mag,

	DT_CustomType = 0x1000'0000,

	DT_PrimaryMask = 0x00FF,
	DT_SubtypeMask = 0xFF00,
	DT_SubtypeShift = 8,
};
Q_DECLARE_FLAGS(DeviceTypes, DeviceType)
Q_FLAG_NS(DeviceTypes)

enum DeviceState : uint8_t {
	DS_Unknown,
	DS_Seen,
	DS_Connected,
	DS_Open,
	DS_Reporting,
	// DS_Error,
};
Q_ENUM_NS(DeviceState)

// Do not change the order and/or consult strings.h when modifying members.
enum EventType : uint8_t
{
	Event_Generic,
	Event_Found,
	Event_Removed,
	Event_Started,
	Event_Stopped,
	Event_Axis,
	Event_Button,
	Event_Hat,
	Event_Key,
	Event_Motion,
	Event_Scroll,
	Event_Sensor,
	Event_HID,

	EVENT_TYPE_ENUM_MAX
};
Q_ENUM_NS(EventType)

enum MouseButton : uint8_t
{
	MB_None = 0,
	MB_Left,
	MB_Right,
	MB_Middle,
};
Q_ENUM_NS(MouseButton)

struct DisplayInfo
{
	short index {0};
	bool isPrimary {false};
	QRect rect {};
	float scale {1.0f};
	std::string name {};
};

// These are only used by the core Plugin to parse key modifier bitfields;
// They're here to avoid needing to include SDL directly, but should
// match the corresponding SDL macros.
enum ModifierKey : uint16_t
{
	MK_NONE   = 0x0000,  /**< SDL_KMOD_NONE   */
	MK_LSHIFT = 0x0001,  /**< SDL_KMOD_LSHIFT */
	MK_RSHIFT = 0x0002,  /**< SDL_KMOD_RSHIFT */
	MK_LEVEL5 = 0x0004,  /**< SDL_KMOD_LEVEL5 */
	MK_LCTRL  = 0x0040,  /**< SDL_KMOD_LCTRL  */
	MK_RCTRL  = 0x0080,  /**< SDL_KMOD_RCTRL  */
	MK_LALT   = 0x0100,  /**< SDL_KMOD_LALT   */
	MK_RALT   = 0x0200,  /**< SDL_KMOD_RALT   */
	MK_LGUI   = 0x0400,  /**< SDL_KMOD_LGUI   */
	MK_RGUI   = 0x0800,  /**< SDL_KMOD_RGUI   */
	MK_NUM    = 0x1000,  /**< SDL_KMOD_NUM    */
	MK_CAPS   = 0x2000,  /**< SDL_KMOD_CAPS   */
	MK_MODE   = 0x4000,  /**< SDL_KMOD_MODE   */
	MK_SCROLL = 0x8000,  /**< SDL_KMOD_SCROLL */
	MKC_SHIFT  = MK_LSHIFT | MK_RSHIFT,  /**< SDL_KMOD_SHIFT */
	MKC_CTRL   = MK_LCTRL  | MK_RCTRL,   /**< SDL_KMOD_CTRL  */
	MKC_ALT    = MK_LALT   | MK_RALT,    /**< SDL_KMOD_ALT   */
	MKC_GUI    = MK_LGUI   | MK_RGUI,    /**< SDL_KMOD_GUI   */
};
Q_DECLARE_FLAGS(ModifierKeys, ModifierKey)
Q_FLAG_NS(ModifierKeys)

// SDL_SCANCODE_LCTRL = 224,
// SDL_SCANCODE_LSHIFT = 225,
// SDL_SCANCODE_LALT = 226,
// SDL_SCANCODE_LGUI = 227,
// SDL_SCANCODE_RCTRL = 228,
// SDL_SCANCODE_RSHIFT = 229,
// SDL_SCANCODE_RALT = 230,
// SDL_SCANCODE_RGUI = 231,
// SDL_SCANCODE_CAPSLOCK = 57,
// SDL_SCANCODE_SCROLLLOCK = 71,
// SDL_SCANCODE_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
// SDL_SCANCODE_MODE = 257,

// Also used by plugin, same notes as above about matching SDL enum values.
static inline constexpr bool isModifierKey(uint scancode) {
	return scancode >= 224 && scancode <= 231;
}

static inline constexpr ModifierKey scanCodeToGeneralModifierType(uint scancode) {
	switch (scancode)
	{
		case 224:  // SDL_SCANCODE_LCTRL
		case 228:  // SDL_SCANCODE_RCTRL
			return MKC_CTRL;
		case 225:  // SDL_SCANCODE_LSHIFT
		case 229:  // SDL_SCANCODE_RSHIFT
			return MKC_SHIFT;
		case 226:  // SDL_SCANCODE_LALT
		case 230:  // SDL_SCANCODE_RALT
			return MKC_ALT;
		case 227:  // SDL_SCANCODE_LGUI
		case 231:  // SDL_SCANCODE_RGUI
			return MKC_GUI;
		case 57:   // SDL_SCANCODE_CAPSLOCK
			return MK_CAPS;
		case 71:   // SDL_SCANCODE_SCROLLLOCK
			return MK_SCROLL;
		case 83:   // SDL_SCANCODE_NUMLOCKCLEAR
			return MK_NUM;
		case 257:  // SDL_SCANCODE_MODE
			return MK_MODE;
		default:
			return MK_NONE;
	}
}

static inline constexpr bool isToggleKey(uint scancode) {
#if defined(Q_OS_DARWIN)
	return scancode == 57 || scancode == 71 || scancode == 257;
#else
	return scancode == 57 || scancode == 71 || scancode == 83 || scancode == 257;
#endif
}

static QString deviceTypeName(Devices::DeviceTypes type)
{
	static const QHash<DeviceTypes, QString> map {
		{  DeviceType::DT_Unknown,      tr("Unknown", "device type") },
		{  DeviceType::DT_Controller,   tr("Controller")  },
		{  DeviceType::DT_Keyboard,     tr("Keyboard")    },
		{  DeviceType::DT_Pointer,      tr("Pointer")     },
		{  DeviceType::DT_Sensor,       tr("Sensor")      },
		{  DeviceType::DT_GenericHID,   tr("HID")         },
		{  DeviceType::DT_JoystickType, tr("Joystick")    },
		{  DeviceType::DT_GamepadType,  tr("Gamepad")     },
		{  DeviceType::DT_ThrottleType, tr("Throttle")    },
		{  DeviceType::DT_WheelType,    tr("Wheel")       },
		{  DeviceType::DT_MouseType,    tr("Mouse")       },
		{  DeviceType::DT_TrackpadType, tr("Trackpad")    },
		{  DeviceType::DT_TouchType,    tr("Touchscreen") },
		{  DeviceType::DT_PenType,      tr("Pen")         },
	};
	static const QString defaultType { map.value(DeviceType::DT_Unknown) };
	return map.value(type, defaultType);
}

static Devices::DeviceTypes deviceTypeNameToType(QStringView name)
{
	static const QHash<QString, DeviceTypes> map {
		{ deviceTypeName(DeviceType::DT_Unknown     ), DeviceType::DT_Unknown      },
		{ deviceTypeName(DeviceType::DT_Controller  ), DeviceType::DT_Controller   },
		{ deviceTypeName(DeviceType::DT_Keyboard    ), DeviceType::DT_Keyboard     },
		{ deviceTypeName(DeviceType::DT_Pointer     ), DeviceType::DT_Pointer      },
		{ deviceTypeName(DeviceType::DT_Sensor      ), DeviceType::DT_Sensor       },
		{ deviceTypeName(DeviceType::DT_GenericHID  ), DeviceType::DT_GenericHID   },
		{ deviceTypeName(DeviceType::DT_JoystickType), DeviceType::DT_JoystickType },
		{ deviceTypeName(DeviceType::DT_GamepadType ), DeviceType::DT_GamepadType  },
		{ deviceTypeName(DeviceType::DT_ThrottleType), DeviceType::DT_ThrottleType },
		{ deviceTypeName(DeviceType::DT_WheelType   ), DeviceType::DT_WheelType    },
		{ deviceTypeName(DeviceType::DT_MouseType   ), DeviceType::DT_MouseType    },
		{ deviceTypeName(DeviceType::DT_TrackpadType), DeviceType::DT_TrackpadType },
		{ deviceTypeName(DeviceType::DT_TouchType   ), DeviceType::DT_TouchType    },
		{ deviceTypeName(DeviceType::DT_PenType     ), DeviceType::DT_PenType      },
	};
	return map.value(name, DeviceType::DT_Unknown);
}

}

Q_DECLARE_METATYPE(Devices::DisplayInfo)
