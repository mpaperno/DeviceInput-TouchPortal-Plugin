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

#include <QTimer>

#include "SDLManager.h"

#include "events.h"
#include "DeviceDescriptor.h"
#include "logging.h"
// #include "version.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hidapi.h>
#include <SDL3/SDL_joystick.h>

#define PLATFORM_SDL_PUMP_INTERVAL_MS       32
#define PLATFORM_SDL_PUMP_IDLE_INTERVAL_MS  2000

using namespace Devices;
using namespace Qt::Literals::StringLiterals;

static Q_LOGGING_CATEGORY(lcSDL, "Devices.SDL", LOGMINLEVEL)

#if 0
void debugHidDeviceInfo(SDL_hid_device_info *devInfo)
{
	qCDebug(lcSDL).nospace()
		<< LOG_HEX(devInfo->vendor_id, 4) << DBG_SEP << LOG_HEX(devInfo->product_id, 4) << DBG_SEP << LOG_HEX(devInfo->release_number, 4) << DBG_SEP
		<< LOG_HEX(devInfo->usage, 4) << DBG_SEP << LOG_HEX(devInfo->usage_page, 4) << DBG_SEP
		<< QString::fromWCharArray(devInfo->manufacturer_string) << DBG_SEP
		<< QString::fromWCharArray(devInfo->product_string) << DBG_SEP
		<< QString::fromWCharArray(devInfo->serial_number) << DBG_SEP
		<< devInfo->path;
}
#endif

#if 0
// \\?\HID#HIDCLASS&Col01#1&2d595ca7&2&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}              vJoy
// \\?\HID#HIDCLASS&Col02#1&2d595ca7&2&0001#{4d1e55b2-f16f-11cf-88cb-001111000030}              vJoy
// \\?\HID#VID_1DD2&PID_100C#6&228b13c2&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}           Pedals adapter
// \\?\HID#VID_10F5&PID_7055&MI_00#7&609bfc7&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}      Velocity FS
// \\?\HID#VID_046D&PID_C216#6&4ab1388&1&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}            Logitech Dual Action
// \\?\HID#VID_045E&PID_028E&IG_00#3&2e05e610&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}     Xbox360
// \\?\HID#VID_045E&PID_028E&IG_00#3&34a1859b&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}
// \\?\HID#VID_045E&PID_028E&IG_03#3&3103e80&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}      Xbox360
// \\?\HID#VID_045E&PID_028E&IG_06#3&21240b46&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}
// \\?\HID#VID_045E&PID_028E&IG_09#3&2572703d&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}
// \\?\HID#VID_054C&PID_05C4&REV_0100#2&18dd2c13&1&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}  PS4
// \\?\HID#VID_054C&PID_05C4&REV_0100#2&2b96dcb8&1&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}  PS4
// \\?\HID#VID_054C&PID_05C4&REV_0100#2&2b5c356&1&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}
static uint findDeviceInstance(const DeviceDescriptor &dd)
{
#ifdef Q_OS_WIN
	uint i;
	bool ok = false;
	auto path = QByteArrayView(dd.hwData.path);

	if (const auto idx = path.indexOf("Col"_ba)) {
		i = path.sliced(idx + 3, 2).toUInt(&ok, 16);
		if (ok)
			return i;
	}

	if (const auto idx = path.indexOf("IG_"_ba)) {
		i = path.sliced(idx + 3, 2).toUInt(&ok, 16);
		if (ok)
			return i / 3 + 1;
	}

	if (const auto idx = path.indexOf("MI_"_ba)) {
		i = path.sliced(idx + 3, 2).toUInt(&ok, 16);
		if (ok)
			return ++i;
	}
#endif
	return dd.instance;
}
#endif

static Devices::DeviceTypes joyDeviceTypeFromSdlType(SDL_JoystickType t)
{
	// qCDebug(lcSDL) << t;
	const DeviceTypes ct = DeviceType::DT_Controller;
	switch(t)
	{
		case SDL_JOYSTICK_TYPE_GAMEPAD:
			return ct | DeviceType::DT_Gamepad;
    case SDL_JOYSTICK_TYPE_WHEEL:
			return ct | DeviceType::DT_Wheel;
		case SDL_JOYSTICK_TYPE_THROTTLE:
			return ct | DeviceType::DT_Throttle;
    case SDL_JOYSTICK_TYPE_ARCADE_STICK:
    case SDL_JOYSTICK_TYPE_FLIGHT_STICK:
			return ct | DeviceType::DT_Joystick;
    // case SDL_JOYSTICK_TYPE_DANCE_PAD:
    // case SDL_JOYSTICK_TYPE_GUITAR:
    // case SDL_JOYSTICK_TYPE_DRUM_KIT:
    // case SDL_JOYSTICK_TYPE_ARCADE_PAD:
		default:
			return ct /*| DeviceType::DT_Joystick*/;
	}
}

static inline bool areGuidEqual(const SDL_GUID &l, const SDL_GUID &r) {
	return !memcmp(l.data, r.data, sizeof(SDL_GUID));
}


//
// SDLManagerPrivate
//

class SDLManagerPrivate
{
	Q_DECLARE_PUBLIC(SDLManager)
	public:

	static bool guidIsZero(const SDL_GUID &id) {
		static const constexpr SDL_GUID GUID_ZERO {};
		return areGuidEqual(id, GUID_ZERO);
	}

	SDLManagerPrivate(SDLManager *q) :
	  q_ptr(q)
	{
		tickTim.setTimerType(Qt::PreciseTimer);
		QObject::connect(&tickTim, &QTimer::timeout, SDL_PumpEvents);
	}

	DeviceDescriptor ddFromJoystickId(uint id) const
	{
		DeviceDescriptor dd { DeviceAPI::DA_SDL };

		const SDL_GUID guid = SDL_GetJoystickGUIDForID(id);
		if (guidIsZero(guid)) {
			q_ptr->setLastError(u"Got a ZERO GUID for joystick instance_id %1: %2"_s.arg(id).arg(QByteArray::fromRawData((const char *)guid.data, sizeof(SDL_GUID)).toHex(':')));
			qCWarning(lcSDL) << q_ptr->getLastError();
			return dd;
		}
		// Get a more specific SDL device type
		const SDL_JoystickType jtype = SDL_GetJoystickTypeForID(id);

		dd.apiId = id;
		dd.type = joyDeviceTypeFromSdlType(jtype);

		// Parse GUID info into hardware device details
		SDL_GetJoystickGUIDInfo(guid, &dd.hwData.VID, &dd.hwData.PID, &dd.hwData.version, nullptr);
		dd.hwData.path = SDL_GetJoystickPathForID(id);

		// Try get more info with HID driver
		if (SDL_hid_device_info *hidDevs = SDL_hid_enumerate(dd.hwData.VID, dd.hwData.PID)) {
			// find device that matches current path exactly
			for (SDL_hid_device_info *devInfo = hidDevs; devInfo; devInfo = devInfo->next) {
				// debugHidDeviceInfo(devInfo);
				if (QByteArray(devInfo->path).toUpper() == dd.hwData.path.toUpper()) {
					dd.hwData.path = devInfo->path;
					dd.hwData.version = devInfo->release_number;  // more reliable than SDL default detection
					dd.hwData.vendor = QString::fromWCharArray(devInfo->manufacturer_string);
					dd.hwData.serial = QString::fromWCharArray(devInfo->serial_number);
					if (jtype != SDL_JOYSTICK_TYPE_GAMEPAD)
						dd.hwData.product = QString::fromWCharArray(devInfo->product_string);
					break;
				}
			}
			SDL_hid_free_enumeration(hidDevs);
		}

		if (dd.hwData.product.isEmpty())
			dd.hwData.product = SDL_GetJoystickNameForID(id);

		dd.uid = dd.hwData.path;
		dd.name = dd.hwData.product;

		// dd.guid = QUuid::fromBytes(guid.data);
		// char sGuid[33];
		// SDL_GUIDToString(guid, sGuid, sizeof(sGuid));
		// qCDebug(lcSDL) << sGuid;

		if (dd.uid.isEmpty())
			dd.uid = dd.name.toLatin1() + '-' + QByteArray::number(dd.apiId, 16).toUpper();

		// if (jtype == SDL_JOYSTICK_TYPE_GAMEPAD) {
		// 		if (const int playerIdx = SDL_GetGamepadPlayerIndexForID(id); playerIdx > -1)
		// 			dd.instance = playerIdx + 1;
		// 		qCDebug(lcSDL) << "Got Gamepad Player ID?" << dd.instance;
		// }

		// dd.instance = findDeviceInstance(dd);

		return dd;
	}

	bool SDLEventHander(SDL_Event *event)
	{
		uint instanceId = 0;
		const DeviceDescriptor *dd = nullptr;
		DeviceEvent *ev = nullptr;
		// DeviceDescriptor ddd;

		switch(event->type)
		{
			// Joystick

			case SDL_EVENT_JOYSTICK_ADDED:
				// dd =
				addDiscoveredJoystick(event->jdevice.which);
				break;

			case SDL_EVENT_JOYSTICK_REMOVED:
				removeDiscoveredJoystick(event->jdevice.which /*, &ddd*/);
				// dd = &ddd;
				break;

			case SDL_EVENT_JOYSTICK_AXIS_MOTION:
				ev = new DeviceAxisEvent(event->common.timestamp, event->jaxis.axis + 1, std::clamp((event->jaxis.value - -32768) * (1.0f / 65535), 0.0f, 1.0f));
				break;

			case SDL_EVENT_JOYSTICK_HAT_MOTION:
				ev = new DeviceHatEvent(event->common.timestamp, event->jhat.hat + 1, event->jhat.value);
				break;

			case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
			case SDL_EVENT_JOYSTICK_BUTTON_UP:
				ev = new DeviceButtonEvent(event->common.timestamp, event->jbutton.button + 1, event->jbutton.down);
				break;

			case SDL_EVENT_JOYSTICK_BALL_MOTION:
				ev = new DeviceScrollEvent(event->common.timestamp, event->jball.ball + 1, (float)event->jball.xrel, (float)event->jball.yrel);
				break;

			case SDL_EVENT_JOYSTICK_UPDATE_COMPLETE:
				return true;  // ignore

			// Gamepad?
			// case SDL_EVENT_GAMEPAD_ADDED:
			// case SDL_EVENT_GAMEPAD_REMOVED:
			// case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			// case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			// case SDL_EVENT_GAMEPAD_BUTTON_UP:

			default:
				qCDebug(lcSDL) << "SDL Event" << LOG_HEX(event->type, 8);
				return false;
		}

		if (!ev)
			return true;

		if (!instanceId)
			instanceId = event->jdevice.which;

		if (!dd && knownJoysticks.contains(instanceId))
			dd = &knownJoysticks[instanceId];

		if (dd && dd->type != DeviceType::DT_Unknown) {
			ev->deviceType = dd->type;
			ev->deviceUid = dd->uid;

			// qCDebug(lcSDL) << "Dispatch Event:" << *ev << " || Device:" << instanceId << dd->name;
			Q_EMIT q_ptr->deviceEvent(ev);
			return true;
		}

		// qCDebug(lcSDL) << event->common.timestamp << "||" << LOG_HEX(event->type, 8) << " || Device:" << instanceId;
		delete ev;
		return false;
	}

	bool tryGetDevice(const QByteArray &uid, const DeviceDescriptor *&dd, bool report = true) const
	{
		const uint id = deviceUidMap.value(uid, 0);
		if (!id || !knownJoysticks.contains(id)) {
			if (report) {
				q_ptr->setLastError(u"Couldn't find device for UID %1"_s.arg(uid));
				qCWarning(lcSDL) << q_ptr->getLastError();
			}
			return false;
		}
		dd = &const_cast<SDLManagerPrivate*>(this)->knownJoysticks[id];
		if (dd->apiId > 0)
			return true;

		if (report) {
			q_ptr->setLastError(u"Invalid API ID %1 for device UID %2"_s.arg(dd->apiId).arg(uid));
			qCWarning(lcSDL) << q_ptr->getLastError();
		}
		return false;
	}

	bool validateDevice(const DeviceDescriptor &dd, bool checkKnown = true) const
	{
		if (checkKnown && !knownJoysticks.contains(dd.apiId)) {
			q_ptr->setLastError(u"Couldn't find device for UID %1"_s.arg(dd.uid));
			qCWarning(lcSDL) << q_ptr->getLastError();
			return false;
		}
		if (!dd.apiId) {
			q_ptr->setLastError(u"Invalid API ID %1 for device UID %2"_s.arg(dd.apiId).arg(dd.uid));
			qCWarning(lcSDL) << q_ptr->getLastError();
			return false;
		}
		return true;
	}

	const DeviceDescriptor *addDiscoveredJoystick(uint id)
	{
		static const DeviceDescriptor emptyDD;

		if (knownJoysticks.contains(id)) {
			qCDebug(lcSDL) << "Device:" << id << "already exists.";
			return &knownJoysticks[id];
		}

		const DeviceDescriptor dd = ddFromJoystickId(id);
		if (dd.type == DeviceType::DT_Unknown)
			return nullptr;

		knownJoysticks.insert(id, dd);
		deviceUidMap.insert(dd.uid, id);

		qCDebug(lcSDL) << "Added New Device:" << dd;
		Q_EMIT q_ptr->deviceDiscovered(dd);

		return &knownJoysticks[id];
	}

	bool removeDiscoveredJoystick(uint id, DeviceDescriptor *ddd = nullptr)
	{
		Q_Q(SDLManager);
		if (const DeviceDescriptor &dd = knownJoysticks.value(id); dd.apiId > 0) {
			if (!shuttingDown)
				q->disconnectDevice(dd.uid);

			qCDebug(lcSDL) << "Joystick device removed, UID: " << dd.uid;
			Q_EMIT q->deviceRemoved(dd.uid);

			knownJoysticks.remove(id);
			deviceUidMap.remove(dd.uid);

			if (ddd)
				*ddd = std::move(dd);
			return true;
		}
		return false;
	}

	void discoverDevices()
	{
		int count = 0;
		SDL_JoystickID *joyList = SDL_GetJoysticks(&count);
		if (!joyList) {
			qCWarning(lcSDL) << "Couldn't get list of SDL joysticks:" << SDL_GetError();
			return;
		}
		for (SDL_JoystickID *joyId = joyList; joyId && *joyId; ++joyId) {
			addDiscoveredJoystick(*joyId);
		}
		SDL_free(joyList);

		// if (SDL_hid_device_info *hidDevs = SDL_hid_enumerate(0, 0)) {
		// 	for (SDL_hid_device_info *devInfo = hidDevs; devInfo; devInfo = devInfo->next)
		// 		debugHidDeviceInfo(devInfo);
		// 	SDL_hid_free_enumeration(hidDevs);
		// }

		// enumerateDisplays();
	}

	void enumerateDisplays()
	{
		const auto prevCount = screenInfoList.size();
		screenInfoList.clear();

		bool wasVidInit = (SDL_WasInit(0) & SDL_INIT_VIDEO);
		if (!wasVidInit) {
			if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
				qCWarning(lcSDL) << "Cannot init VIDEO subsystem; SDL error:" << SDL_GetError();
				return;
			}
		}

		int nDisplays;
		SDL_DisplayID *displays = SDL_GetDisplays(&nDisplays);
		screenInfoList.reserve(nDisplays);

		const auto primaryId = SDL_GetPrimaryDisplay();
		QRect desktopRect;
		SDL_Rect rect;
		for(int i = 0; i < nDisplays; i++) {
			const auto &dispId = displays[i];
			if (!SDL_GetDisplayBounds(dispId, &rect))
				continue;

			const float scl = SDL_GetDisplayContentScale(dispId);
			const DisplayInfo si { short(i + 1) /*dispId*/, primaryId == dispId, QRect(rect.x, rect.y, rect.w, rect.h), (scl > 0.0f ? scl : 1.0f), SDL_GetDisplayName(dispId) };
			desktopRect |= si.rect;

			qCDebug(lcSDL) << "Got monitor info:" << si.index << "out of" << nDisplays << "Rect:" << si.rect << "Scale:" << si.scale << "Primary?" << si.isPrimary << "Name;" << si.name;
			Q_EMIT q_ptr->displayDetected(si);
			screenInfoList.append(std::move(si));
		}
		SDL_free(displays);

		if (!desktopRect.isEmpty()) {
			// const ScreenInfo si { 0, false, true, desktopRect, 1.0f, Devices::tr("Desktop").toStdString() };
			qCDebug(lcSDL) << "Virtual Desktop Rect:" << desktopRect;
			// screenInfoList.append(std::move(si));
			Q_EMIT q_ptr->displayDetected({ 0, false, desktopRect, 1.0f, Devices::tr("Desktop").toStdString() });
		}

		for (auto i = screenInfoList.size(); i < prevCount; ++i) {
			Q_EMIT q_ptr->displayRemoved(short(i + 1));
		}

		if (!wasVidInit)
			SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}

	void updateDeviceReport(const DeviceDescriptor &dd)
	{
		// const DeviceDescriptor &dd = knownJoysticks.value(id);
		if (dd.type == DeviceType::DT_Unknown)
			return;

		SDL_Event event;
		event.common.timestamp = SDL_GetTicksNS();

		if (dd.type & DeviceType::DT_Controller) {

			bool wasClosed = false;
			SDL_Joystick *joy = SDL_GetJoystickFromID(dd.apiId);
			if (!joy) {
				joy = SDL_OpenJoystick(dd.apiId);
				wasClosed = true;
			}
			if (!joy) {
				qCWarning(lcSDL) << "Couldn't open joystick" << dd.uid << SDL_GetError();
				return;
			}

			SDL_UpdateJoysticks();

			event.jdevice.which = dd.apiId;

			int n = SDL_GetNumJoystickAxes(joy);
			for (int i=0; i < n; ++i) {
				// const uint16_t val = SDL_GetJoystickAxis(joy, i);
				event.type = SDL_EVENT_JOYSTICK_AXIS_MOTION;
				event.jaxis.axis = i;
				event.jaxis.value = SDL_GetJoystickAxis(joy, i);
				// qCDebug(lcSDL) << "Sending Joystick axis event: " << event.jaxis.which << event.jaxis.axis << event.jaxis.value;
				// SDL_PushEvent(&event);
				SDLEventHander(&event);
			}

			n = SDL_GetNumJoystickHats(joy);
			for (int i=0; i < n; ++i) {
				// const uint8_t val = SDL_GetJoystickHat(joy, i);
				// if (!val)
				// 	continue;
				event.type = SDL_EVENT_JOYSTICK_HAT_MOTION;
				event.jhat.hat = i;
				event.jhat.value = SDL_GetJoystickHat(joy, i);
				// qCDebug(lcSDL) << "Sending Joystick hat event: " << event.jdevice.which << event.jhat.hat << event.jhat.value;
				// SDL_PushEvent(&event);
				SDLEventHander(&event);
			}

			n = SDL_GetNumJoystickButtons(joy);
			for (int i=0; i < n; ++i) {
				const bool dn = SDL_GetJoystickButton(joy, i);
				// only report the first 32 buttons, unless this one is actually pressed
				if (!dn && i > 32)
					continue;
				event.type = SDL_EVENT_JOYSTICK_BUTTON_DOWN;
				event.jbutton.button = i;
				event.jbutton.down = dn;
				// SDL_PushEvent(&event);
				SDLEventHander(&event);
			}

			n = SDL_GetNumJoystickBalls(joy);
			for (int i=0; i < n; ++i) {
				int dx, dy;
				if (!SDL_GetJoystickBall(joy, i, &dx, &dy) /*|| (dx == 0 && dy == 0)*/)
					continue;
				event.type = SDL_EVENT_JOYSTICK_BALL_MOTION;
				event.jball.ball = i;
				event.jball.xrel = dx;
				event.jball.yrel = dy;
				// SDL_PushEvent(&event);
				SDLEventHander(&event);
			}

			// SDL_PumpEvents();

			if (wasClosed)
				SDL_CloseJoystick(joy);
		}
	}

	void checkForRemovedDevices()
	{
		const auto current = knownJoysticks.values();
		for (const auto &dd : current) {
			bool exists = false;

			if ((dd.type & DeviceType::DT_Controller) && dd.apiId > 0) {
				exists = !guidIsZero(SDL_GetJoystickGUIDForID(dd.apiId));
			}

			if (!exists) {
				removeDiscoveredJoystick(dd.apiId);
				qCDebug(lcSDL) << "Stored device no longer exists, removing" << dd.uid;
			}
		}
	}

	void disconnectAllDevicesQuietly()
	{
		Q_Q(SDLManager);
		for (const auto &dd : knownJoysticks.values())
			q->disconnectDevice(dd.uid);
	}

	void closeSDL()
	{
		if (!sdlInit || shuttingDown)
			return;

		shuttingDown = true;
		disconnectAllDevicesQuietly();
		tickTim.stop();
		// SDL_hid_exit();
		SDL_Quit();
		sdlInit = false;
	}

	std::atomic_bool sdlInit { false };
	std::atomic_bool initializing { false };
	std::atomic_bool shuttingDown { false };
	std::atomic_uint_fast32_t numConnectedDevices { 0 };
	int pumpTimerInterval { PLATFORM_SDL_PUMP_INTERVAL_MS };
	int idleTimerInterval { PLATFORM_SDL_PUMP_IDLE_INTERVAL_MS };
	QHash<uint, DeviceDescriptor> knownJoysticks;
	QHash<QByteArray, uint> deviceUidMap;
	QList<DisplayInfo> screenInfoList;
	QString lastError;
	QTimer tickTim;
	SDLManager * const q_ptr;
};

static bool SDLCALL SDLEventHander(void *context, SDL_Event *event)
{
	static_cast<SDLManagerPrivate *>(context)->SDLEventHander(event);
	return true;
}


//
// SDLManager
//

SDLManager::SDLManager(QObject *parent) :
  IApiManager{parent},
  d_ptr(new SDLManagerPrivate(this))
{
	// SDL_SetAppMetadata(PLUGIN_NAME, APP_VERSION_STR, PLUGIN_ID);
	SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1", SDL_HINT_OVERRIDE);
	SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_ENHANCED_REPORTS, "0", SDL_HINT_OVERRIDE);
	SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_THREAD, "1", SDL_HINT_OVERRIDE);
	SDL_SetHintWithPriority(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1", SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_XINPUT_ENABLED, "0", SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_DIRECTINPUT, "0", SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_RAWINPUT, "0", SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_RAWINPUT_CORRELATE_XINPUT, "0", SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_HIDAPI, "0", SDL_HINT_OVERRIDE);
	SDL_SetHint(SDL_HINT_HIDAPI_ENUMERATE_ONLY_CONTROLLERS, "0");
	SDL_SetHint(SDL_HINT_JOYSTICK_FLIGHTSTICK_DEVICES,
		"0x10F5/0x7055,"  // Turtle Beach VelocityOne Flightstick
		"0x1234/0xBEAD,"  // vJoy
	);
}

SDLManager::~SDLManager()
{
	deinit();
	delete d_ptr;
}

bool SDLManager::init()
{
	Q_D(SDLManager);
	if (d->sdlInit || d->initializing)
		return true;

	d->initializing = true;

	SDL_InitFlags sdlFlags = SDL_INIT_JOYSTICK /*| SDL_INIT_GAMEPAD*/;
#if PLATFORM_SDL_INIT_VIDEO
	sdlFlags |= SDL_INIT_VIDEO;
#endif
	if (!SDL_Init(sdlFlags)) {
		qCCritical(lcSDL) << "Couldn't initialize SDL:" << SDL_GetError();
	}
	else if (!SDL_AddEventWatch(SDLEventHander, d)) {
		qCCritical(lcSDL) << "Couldn't subscribe to SDL Events:" << SDL_GetError();
	}
	else {
		if (SDL_hid_init())
			qCWarning(lcSDL) << "HID Init error:" << SDL_GetError();

		SDL_SetEventEnabled(SDL_EVENT_JOYSTICK_UPDATE_COMPLETE, false);
		SDL_SetEventEnabled(SDL_EVENT_JOYSTICK_BATTERY_UPDATED, false);
		SDL_SetEventEnabled(SDL_EVENT_GAMEPAD_ADDED, false);
		SDL_SetEventEnabled(SDL_EVENT_GAMEPAD_REMOVED, false);
		SDL_SetEventEnabled(SDL_EVENT_GAMEPAD_AXIS_MOTION, false);
		SDL_SetEventEnabled(SDL_EVENT_GAMEPAD_BUTTON_DOWN, false);
		SDL_SetEventEnabled(SDL_EVENT_GAMEPAD_BUTTON_UP, false);
		SDL_SetEventEnabled(SDL_EVENT_GAMEPAD_UPDATE_COMPLETE, false);
		SDL_SetEventEnabled(SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED, false);

		if (d->idleTimerInterval)
			d->tickTim.start(d->idleTimerInterval);
		clearLastError();
		d->sdlInit = true;
	}

	d->initializing = false;
	return d->sdlInit;
}


void SDLManager::deinit()
{
	d_ptr->closeSDL();
}

QString SDLManager::getLastError() const
{
	if (m_lastError.isEmpty())
		return SDL_GetError();
	return m_lastError;
}

int SDLManager::activeScanInterval() const {
	return d_ptr->pumpTimerInterval;
}

int SDLManager::defaultActiveScanInterval() const {
	return PLATFORM_SDL_PUMP_INTERVAL_MS;
}


void SDLManager::setActiveScanInterval(int ms)
{
	Q_D(SDLManager);
	if (ms == d->pumpTimerInterval)
		return;

	d->pumpTimerInterval = std::max(0, ms);

	if (!d->numConnectedDevices)
		return;

	if (ms <= 0)
		d->tickTim.stop();
	else
		d->tickTim.start(d->pumpTimerInterval);
}

int SDLManager::idleScanInterval() const {
	return d_ptr->idleTimerInterval;
}

int SDLManager::defaultIdleScanInterval() const {
	return PLATFORM_SDL_PUMP_IDLE_INTERVAL_MS;
}

void SDLManager::setIdleScanInterval(int ms)
{
	Q_D(SDLManager);
	if (ms == d->idleTimerInterval)
		return;

	d->idleTimerInterval = std::max(0, ms);

	if (!!d->numConnectedDevices)
		return;

	if (ms <= 0)
		d->tickTim.stop();
	else
		d->tickTim.start(d->idleTimerInterval);
}

void SDLManager::scanDevices()
{
	Q_D(SDLManager);
	if (!d->sdlInit)
		return;

	SDL_UpdateJoysticks();
	SDL_PumpEvents();
	d->checkForRemovedDevices();
	d->knownJoysticks.clear();
	d->discoverDevices();
	SDL_PumpEvents();
}

void SDLManager::connectDevice(const QByteArray &uid)
{
	Q_D(SDLManager);
	const DeviceDescriptor *dd = nullptr;
	if (!d->tryGetDevice(uid, dd)) {
		setLastError(u"Device not found for UID %1"_s.arg(uid));
		return;
	}
	const bool prevConnected = d->numConnectedDevices > 0;

	switch(dd->type & DeviceType::DT_PrimaryMask)
	{
		case DeviceType::DT_Controller:
			if (SDL_GetJoystickFromID(dd->apiId))
				break; // already opened

			if (SDL_OpenJoystick(dd->apiId)) {
				++d->numConnectedDevices;
				qCDebug(lcSDL) << "Opened Joystick Instance ID:" << dd->apiId << dd->name << dd->uid;
				break;
			}
			setLastError(u"Can't open Device Instance ID: %1; Name: %2; SDL Error: %3"_s.arg(Devices::deviceTypeName(dd->type), dd->name, SDL_GetError()));
			qCWarning(lcSDL) << getLastError();
			return;

		default:
			setLastError(u"Can't handle device type: %1"_s.arg(Devices::deviceTypeName(dd->type)));
			qCWarning(lcSDL) << getLastError();
			return;
	}

	if (!prevConnected && d->numConnectedDevices > 0 && d->pumpTimerInterval > 0) {
		d->tickTim.start(d->pumpTimerInterval);
		qCDebug(lcSDL) << "First active device connection, starting SDL event loop at full speed now.";
	}

	clearLastError();
	Q_EMIT deviceReportToggled(uid, true);
}

void SDLManager::disconnectDevice(const QByteArray &uid)
{
	Q_D(SDLManager);

	const DeviceDescriptor *dd = nullptr;
	if (!d->tryGetDevice(uid, dd))
		return;

	switch(dd->type & DeviceType::DT_PrimaryMask)
	{
		case DeviceType::DT_Controller:
			if (SDL_Joystick *joy = SDL_GetJoystickFromID(dd->apiId)) {
				SDL_CloseJoystick(joy);
				if (d->numConnectedDevices > 0)
					--d->numConnectedDevices;
				qCDebug(lcSDL) << "Controller device disconnected:" << dd->apiId << Devices::deviceTypeName(dd->type) << dd->name << dd->uid;
				break;
			}
			// Not connected or doesn't exist
			return;

		default:
			qCWarning(lcSDL) << "Can't handle device type: " << dd->type;
			return;
	}

	clearLastError();
	Q_EMIT deviceReportToggled(uid, false);

	if (!d->numConnectedDevices) {
		if (d->idleTimerInterval > 0)
			d->tickTim.start(d->idleTimerInterval);
		else
			d->tickTim.stop();
		qCDebug(lcSDL) << "No more connected devices, slowing SDL event loop now.";
	}
}

void SDLManager::sendDeviceReport(const QByteArray &uid)
{
	Q_D(SDLManager);
	if (uid == DI_SYSTEM_SCREEN_UID) {
		d->enumerateDisplays();
		return;
	}

	const DeviceDescriptor *dd = nullptr;
	if (d->tryGetDevice(uid, dd))
		d->updateDeviceReport(*dd);
}

#include "moc_SDLManager.cpp"
