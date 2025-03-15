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

#include <QByteArray>
#include <QHash>

#include "devices.h"
#include "version.h"

#define PLUGIN_STR_PATH_SEP           "."
#define PLUGIN_STR_EV_STATE_DEVICE    "InputDevice"
#define PLUGIN_STR_EV_STATE_RUNSTATE  PLUGIN_SYSTEM_NAME PLUGIN_STR_PATH_SEP "RunState"
#define PLUGIN_STR_EV_STATE_PGCHANGE  PLUGIN_SYSTEM_NAME PLUGIN_STR_PATH_SEP "PageChange"
#define PLUGIN_STR_CAT_DEVICES_NAME   "Device Information"
#define PLUGIN_STR_CAT_ASSIGNED_NAME  "Device Assignments"
#define PLUGIN_STR_STATEID_DEVICES    "devices"
#define PLUGIN_STR_STATEID_DISPLAY    "display"
#define PLUGIN_STR_STATEID_ASSIGNED   "assigned"
#define PLUGIN_STR_STATEID_DEFAULT    "default"
#define PLUGIN_STR_STATEID_FIRST      "first"

#define PLUGIN_STR_MISC_ACT_DATA_PLACEHOLDER_PFX  "select "

namespace Strings {

Q_NAMESPACE

static inline QString tr(const char *sourceText, const char *disambiguation = nullptr, int n = -1) {
	return QCoreApplication::translate("Plugin", sourceText, disambiguation, n);
}

enum StateIdToken : uint8_t
{
	SID_DevicesList,
	SID_ReportingDevices,
	SID_LastError,
	SID_ErrorCount,
	SID_PluginState,
	SID_DeviceStatusChange,
	SID_LastFoundDevice,
	SID_LastRemovedDevice,
	SID_DevReportStarted,
	SID_DevReportStopped,
	SID_Display,
	SID_DisplaysCount,
	SID_DisplayPrimary,
	SID_TpCurrentPage,

	SID_KBDMOD_FIRST,
	SID_KbdModShift = SID_KBDMOD_FIRST,
	SID_KbdModCtrl,
	SID_KbdModAlt,
	SID_KbdModGui,
	SID_KbdModCapLock,
	SID_KbdModScrLock,
	SID_KbdModNumLock,
	SID_KBDMOD_LAST = SID_KbdModNumLock,

	SID_ENUM_MAX
};

static constexpr const char * g_stateIdTokenStrings[SID_ENUM_MAX]
{
	"deviceList",
	"reportingDeviceList",
	"lastError",
	"errorCount",
	"pluginState",
	"deviceStatusChange",
	"lastAddedDevice",
	"lastRemovedDevice",
	"deviceReportStarted",
	"deviceReportStopped",
	PLUGIN_STR_STATEID_DISPLAY,
	PLUGIN_STR_STATEID_DISPLAY PLUGIN_STR_PATH_SEP "count",
	PLUGIN_STR_STATEID_DISPLAY PLUGIN_STR_PATH_SEP "primary",
	"currentPage",

	"kbd.mod.shift",
	"kbd.mod.ctrl",
	"kbd.mod.alt",
	"kbd.mod.gui",
	"kbd.mod.caplock",
	"kbd.mod.scrlock",
	"kbd.mod.numlock",
};
static inline constexpr const QLatin1StringView stateIdTokenString(int id) { return QLatin1StringView(g_stateIdTokenStrings[id]); }

// enum ActionIdToken
// {
// 	AID_PluginControl = SID_ENUM_MAX,
// 	AID_DeviceControl,

// 	AID_ENUM_MAX
// };

// enum ActionDataIdToken
// {
// 	ADID_ENUM_FIRST = AID_ENUM_MAX,
// 	ADID_ENUM_MAX
// };

enum EventIdToken : uint8_t
{
	EID_DeviceFound,
	EID_DeviceRemoved,
	EID_ReportStarted,
	EID_ReportStopped,
	// EID_DeviceStatusChange,  // not dispatched, reacts to SID_DeviceStatusChange state change
	EID_PluginStateChanged,
	EID_TpCurrentPageChange,

	EID_DeviceEvent,  // discovered/removed/started/stopped
	EID_DeviceAxis,
	EID_DeviceButton,
	EID_DeviceHat,
	EID_DeviceKey,
	EID_DeviceMotion,
	EID_DeviceScroll,
	EID_DeviceSensor,
	EID_DeviceHIDReport,

	EID_ENUM_MAX,
	EID_ENUM_FIRST = SID_ENUM_MAX,
};

static constexpr const char * g_eventIdTokenStrings[EID_ENUM_MAX]
{
	"deviceAdded",
	"deviceRemoved",
	"reportStarted",
	"reportStopped",
	// "deviceStatusChange",
	"pluginState",
	"pageChange",

	"deviceEvent",
	"deviceAxis",
	"deviceButton",
	"deviceHat",
	"deviceKey",
	"deviceMotion",
	"deviceScroll",
	"deviceSensor",
	"deviceHidReport",
};
static inline constexpr const QLatin1StringView eventIdTokenString(int id) { return QLatin1StringView(g_eventIdTokenStrings[id]); }


enum ChoiceListIdToken : uint8_t
{
	CLID_DeviceCtrDevName,
	CLID_DeviceFilterDevName,
	CLID_DefaultDeviceDevName,

	CLID_ENUM_MAX
};

static constexpr const char * g_choiceListTokenStrings[CLID_ENUM_MAX]
{
	"device.device",
	"filter.device",
	"default.device",
};
static inline const char * const * choiceListTokenStrings() { return g_choiceListTokenStrings; }


static constexpr const char * g_deviceEventStrings[Devices::EventType::EVENT_TYPE_ENUM_MAX]
{
	"Generic",
	"Found",
	"Removed",
	"Started",
	"Stopped",
	"Axis",
	"Button",
	"Hat",
	"Key",
	"Motion",
	"Scroll",
	"Sensor",
	"HID",
};
static inline const char * const * deviceEventStrings() { return g_deviceEventStrings; }

static constexpr const char * g_deviceEventStateIds[Devices::EventType::EVENT_TYPE_ENUM_MAX]
{
	"",
	"found",
	"removed",
	"started",
	"stopped",
	"axis",
	"button",
	"hat",
	"key",
	"motion",
	"scroll",
	"sensor",
	"hid",
};
static inline const char * const * deviceEventStateIds() { return g_deviceEventStateIds; }
static inline constexpr const QLatin1StringView deviceEventStateIdString(int id) { return QLatin1StringView(g_deviceEventStateIds[id]); }

enum ActionTokens : uint8_t
{
	AT_Unknown = 0,

	AID_PluginControl,
	AID_DeviceControl,
	AID_DeviceFilter,
	AID_DeviceDefault,

	CA_RescanDevices,
	CA_FullStatusUpdate,
	CA_DisplaysReport,
	CA_SetControllerRepRate,
	CA_Shutdown,  // dev build only
	CA_StartReport,
	CA_StopReport,
	CA_ToggleReport,
	CA_RefreshReport,
	CA_ClearFilter,

	ST_SendReportStates,
	ST_SendReportEvents,
	// ST_SettingsVersion,

	// send only
	AT_Starting,
	AT_Started,
	AT_Stopped,
	AT_Found,
	AT_Removed,

	AT_Default,
	AT_First,
	AT_AllReportingDevices,
	AT_RemoveDeviceAssignment,
	// AT_FirstController,
	// AT_FirstJoystick,
	// AT_FirstGamepad,
	// AT_FirstThrottle,
	// AT_Increment,
	// AT_Decrement,

	AT_ENUM_MAX
};


static constexpr const char * g_actionTokenStrings[AT_ENUM_MAX] {
	"Unknown",

	"plugin",
	"device",
	"filter",
	"default",

	"Rescan System Devices",
	"Update All States & Events",
	"Send System Displays Report",
	"Set Controller Reporting Rate",
	"Shutdown",
	"Start Reporting",
	"Stop Reporting",
	"Toggle Reporting",
	"Refresh Report",
	"Clear Report Filter",

	"Send Device Reports as States",
	"Send Device Reports as Events",
	// "Settings Version",

	"Starting",
	"Started",
	"Stopped",
	"Found",
	"Removed",

	"Default",
	"First",
	"All With Active Reports",
	"Remove Assignment",
	// "First Controller",
	// "First Joystick",
	// "First Gamepad",
	// "First Throttle",
	// "Increment",
	// "Decrement",
};

static inline constexpr const QLatin1StringView tokenToName(int token) {
	return QLatin1StringView(g_actionTokenStrings[token]);
}

static int tokenFromName(const QByteArray &name, int deflt = AT_Unknown)
{
	static const QHash<QByteArray, int> hash = {
	  { g_actionTokenStrings[AID_PluginControl],   AID_PluginControl },
	  { g_actionTokenStrings[AID_DeviceControl],   AID_DeviceControl },
	  { g_actionTokenStrings[AID_DeviceFilter],    AID_DeviceFilter },
	  { g_actionTokenStrings[AID_DeviceDefault],   AID_DeviceDefault },

	  { g_actionTokenStrings[CA_RescanDevices],    CA_RescanDevices },
	  { g_actionTokenStrings[CA_FullStatusUpdate], CA_FullStatusUpdate },
	  { g_actionTokenStrings[CA_DisplaysReport],   CA_DisplaysReport },
	  { g_actionTokenStrings[CA_Shutdown],         CA_Shutdown },
	  { g_actionTokenStrings[CA_StartReport],      CA_StartReport },
	  { g_actionTokenStrings[CA_StopReport],       CA_StopReport },
	  { g_actionTokenStrings[CA_ToggleReport],     CA_ToggleReport },
	  { g_actionTokenStrings[CA_RefreshReport],    CA_RefreshReport },
	  { g_actionTokenStrings[CA_ClearFilter],      CA_ClearFilter },

	  // { tokenToName(ST_SettingsVersion),   ST_SettingsVersion },

	  { g_actionTokenStrings[AT_Default],                AT_Default },
	  { g_actionTokenStrings[AT_First],                  AT_First },
	  { g_actionTokenStrings[AT_AllReportingDevices],    AT_AllReportingDevices },
	  { g_actionTokenStrings[AT_RemoveDeviceAssignment], AT_RemoveDeviceAssignment },
	  // { g_actionTokenStrings[AT_FirstController],      AT_FirstController },
	  // { g_actionTokenStrings[AT_FirstJoystick],        AT_FirstJoystick },
	  // { g_actionTokenStrings[AT_FirstGamepad],         AT_FirstGamepad },
	  // { g_actionTokenStrings[AT_FirstThrottle],        AT_FirstThrottle },
	};
	return hash.value(name, deflt);
}

static constexpr const char g_pathSep { PLUGIN_STR_PATH_SEP[0] };

}  // namespace Strings
