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

#include <qt_windows.h>

#include <QGlobalStatic>
#include <QThread>
// #include <iostream>

// #include "SDL3/SDL_hints.h"
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>

#include "events.h"
#include "logging.h"
#include "DeviceDescriptor.h"
#include "WindowsDeviceManager.h"

using namespace Devices;

static Q_LOGGING_CATEGORY(lcNAPI, "Devices.Native", LOGMINLEVEL)

// https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#scan-codes
static const SDL_Scancode windows_scancode_table[] = {
    /*0x00*/ SDL_SCANCODE_UNKNOWN,
    /*0x01*/ SDL_SCANCODE_ESCAPE,
    /*0x02*/ SDL_SCANCODE_1,
    /*0x03*/ SDL_SCANCODE_2,
    /*0x04*/ SDL_SCANCODE_3,
    /*0x05*/ SDL_SCANCODE_4,
    /*0x06*/ SDL_SCANCODE_5,
    /*0x07*/ SDL_SCANCODE_6,
    /*0x08*/ SDL_SCANCODE_7,
    /*0x09*/ SDL_SCANCODE_8,
    /*0x0a*/ SDL_SCANCODE_9,
    /*0x0b*/ SDL_SCANCODE_0,
    /*0x0c*/ SDL_SCANCODE_MINUS,
    /*0x0d*/ SDL_SCANCODE_EQUALS,
    /*0x0e*/ SDL_SCANCODE_BACKSPACE,
    /*0x0f*/ SDL_SCANCODE_TAB,
    /*0x10*/ SDL_SCANCODE_Q,
    /*0x11*/ SDL_SCANCODE_W,
    /*0x12*/ SDL_SCANCODE_E,
    /*0x13*/ SDL_SCANCODE_R,
    /*0x14*/ SDL_SCANCODE_T,
    /*0x15*/ SDL_SCANCODE_Y,
    /*0x16*/ SDL_SCANCODE_U,
    /*0x17*/ SDL_SCANCODE_I,
    /*0x18*/ SDL_SCANCODE_O,
    /*0x19*/ SDL_SCANCODE_P,
    /*0x1a*/ SDL_SCANCODE_LEFTBRACKET,
    /*0x1b*/ SDL_SCANCODE_RIGHTBRACKET,
    /*0x1c*/ SDL_SCANCODE_RETURN,
    /*0x1d*/ SDL_SCANCODE_LCTRL,
    /*0x1e*/ SDL_SCANCODE_A,
    /*0x1f*/ SDL_SCANCODE_S,
    /*0x20*/ SDL_SCANCODE_D,
    /*0x21*/ SDL_SCANCODE_F,
    /*0x22*/ SDL_SCANCODE_G,
    /*0x23*/ SDL_SCANCODE_H,
    /*0x24*/ SDL_SCANCODE_J,
    /*0x25*/ SDL_SCANCODE_K,
    /*0x26*/ SDL_SCANCODE_L,
    /*0x27*/ SDL_SCANCODE_SEMICOLON,
    /*0x28*/ SDL_SCANCODE_APOSTROPHE,
    /*0x29*/ SDL_SCANCODE_GRAVE,
    /*0x2a*/ SDL_SCANCODE_LSHIFT,
    /*0x2b*/ SDL_SCANCODE_BACKSLASH,
    /*0x2c*/ SDL_SCANCODE_Z,
    /*0x2d*/ SDL_SCANCODE_X,
    /*0x2e*/ SDL_SCANCODE_C,
    /*0x2f*/ SDL_SCANCODE_V,
    /*0x30*/ SDL_SCANCODE_B,
    /*0x31*/ SDL_SCANCODE_N,
    /*0x32*/ SDL_SCANCODE_M,
    /*0x33*/ SDL_SCANCODE_COMMA,
    /*0x34*/ SDL_SCANCODE_PERIOD,
    /*0x35*/ SDL_SCANCODE_SLASH,
    /*0x36*/ SDL_SCANCODE_RSHIFT,
    /*0x37*/ SDL_SCANCODE_KP_MULTIPLY,
    /*0x38*/ SDL_SCANCODE_LALT,
    /*0x39*/ SDL_SCANCODE_SPACE,
    /*0x3a*/ SDL_SCANCODE_CAPSLOCK,
    /*0x3b*/ SDL_SCANCODE_F1,
    /*0x3c*/ SDL_SCANCODE_F2,
    /*0x3d*/ SDL_SCANCODE_F3,
    /*0x3e*/ SDL_SCANCODE_F4,
    /*0x3f*/ SDL_SCANCODE_F5,
    /*0x40*/ SDL_SCANCODE_F6,
    /*0x41*/ SDL_SCANCODE_F7,
    /*0x42*/ SDL_SCANCODE_F8,
    /*0x43*/ SDL_SCANCODE_F9,
    /*0x44*/ SDL_SCANCODE_F10,
    /*0x45*/ SDL_SCANCODE_PAUSE,
    /*0x46*/ SDL_SCANCODE_SCROLLLOCK,
    /*0x47*/ SDL_SCANCODE_KP_7,
    /*0x48*/ SDL_SCANCODE_KP_8,
    /*0x49*/ SDL_SCANCODE_KP_9,
    /*0x4a*/ SDL_SCANCODE_KP_MINUS,
    /*0x4b*/ SDL_SCANCODE_KP_4,
    /*0x4c*/ SDL_SCANCODE_KP_5,
    /*0x4d*/ SDL_SCANCODE_KP_6,
    /*0x4e*/ SDL_SCANCODE_KP_PLUS,
    /*0x4f*/ SDL_SCANCODE_KP_1,
    /*0x50*/ SDL_SCANCODE_KP_2,
    /*0x51*/ SDL_SCANCODE_KP_3,
    /*0x52*/ SDL_SCANCODE_KP_0,
    /*0x53*/ SDL_SCANCODE_KP_PERIOD,
    /*0x54*/ SDL_SCANCODE_PRINTSCREEN,
    /*0x55*/ SDL_SCANCODE_UNKNOWN,
    /*0x56*/ SDL_SCANCODE_NONUSBACKSLASH,
    /*0x57*/ SDL_SCANCODE_F11,
    /*0x58*/ SDL_SCANCODE_F12,
    /*0x59*/ SDL_SCANCODE_KP_EQUALS,
    /*0x5a*/ SDL_SCANCODE_UNKNOWN,
    /*0x5b*/ SDL_SCANCODE_UNKNOWN,
    /*0x5c*/ SDL_SCANCODE_INTERNATIONAL6,
    /*0x5d*/ SDL_SCANCODE_UNKNOWN,
    /*0x5e*/ SDL_SCANCODE_UNKNOWN,
    /*0x5f*/ SDL_SCANCODE_UNKNOWN,
    /*0x60*/ SDL_SCANCODE_UNKNOWN,
    /*0x61*/ SDL_SCANCODE_UNKNOWN,
    /*0x62*/ SDL_SCANCODE_UNKNOWN,
    /*0x63*/ SDL_SCANCODE_UNKNOWN,
    /*0x64*/ SDL_SCANCODE_F13,
    /*0x65*/ SDL_SCANCODE_F14,
    /*0x66*/ SDL_SCANCODE_F15,
    /*0x67*/ SDL_SCANCODE_F16,
    /*0x68*/ SDL_SCANCODE_F17,
    /*0x69*/ SDL_SCANCODE_F18,
    /*0x6a*/ SDL_SCANCODE_F19,
    /*0x6b*/ SDL_SCANCODE_F20,
    /*0x6c*/ SDL_SCANCODE_F21,
    /*0x6d*/ SDL_SCANCODE_F22,
    /*0x6e*/ SDL_SCANCODE_F23,
    /*0x6f*/ SDL_SCANCODE_UNKNOWN,
    /*0x70*/ SDL_SCANCODE_INTERNATIONAL2,
    /*0x71*/ SDL_SCANCODE_LANG2,
    /*0x72*/ SDL_SCANCODE_LANG1,
    /*0x73*/ SDL_SCANCODE_INTERNATIONAL1,
    /*0x74*/ SDL_SCANCODE_UNKNOWN,
    /*0x75*/ SDL_SCANCODE_UNKNOWN,
    /*0x76*/ SDL_SCANCODE_F24,
    /*0x77*/ SDL_SCANCODE_LANG4,
    /*0x78*/ SDL_SCANCODE_LANG3,
    /*0x79*/ SDL_SCANCODE_INTERNATIONAL4,
    /*0x7a*/ SDL_SCANCODE_UNKNOWN,
    /*0x7b*/ SDL_SCANCODE_INTERNATIONAL5,
    /*0x7c*/ SDL_SCANCODE_UNKNOWN,
    /*0x7d*/ SDL_SCANCODE_INTERNATIONAL3,
    /*0x7e*/ SDL_SCANCODE_KP_COMMA,
    /*0x7f*/ SDL_SCANCODE_UNKNOWN,
    /*0xe000*/ SDL_SCANCODE_UNKNOWN,
    /*0xe001*/ SDL_SCANCODE_UNKNOWN,
    /*0xe002*/ SDL_SCANCODE_UNKNOWN,
    /*0xe003*/ SDL_SCANCODE_UNKNOWN,
    /*0xe004*/ SDL_SCANCODE_UNKNOWN,
    /*0xe005*/ SDL_SCANCODE_UNKNOWN,
    /*0xe006*/ SDL_SCANCODE_UNKNOWN,
    /*0xe007*/ SDL_SCANCODE_UNKNOWN,
    /*0xe008*/ SDL_SCANCODE_UNKNOWN,
    /*0xe009*/ SDL_SCANCODE_UNKNOWN,
    /*0xe00a*/ SDL_SCANCODE_PASTE,
    /*0xe00b*/ SDL_SCANCODE_UNKNOWN,
    /*0xe00c*/ SDL_SCANCODE_UNKNOWN,
    /*0xe00d*/ SDL_SCANCODE_UNKNOWN,
    /*0xe00e*/ SDL_SCANCODE_UNKNOWN,
    /*0xe00f*/ SDL_SCANCODE_UNKNOWN,
    /*0xe010*/ SDL_SCANCODE_MEDIA_PREVIOUS_TRACK,
    /*0xe011*/ SDL_SCANCODE_UNKNOWN,
    /*0xe012*/ SDL_SCANCODE_UNKNOWN,
    /*0xe013*/ SDL_SCANCODE_UNKNOWN,
    /*0xe014*/ SDL_SCANCODE_UNKNOWN,
    /*0xe015*/ SDL_SCANCODE_UNKNOWN,
    /*0xe016*/ SDL_SCANCODE_UNKNOWN,
    /*0xe017*/ SDL_SCANCODE_CUT,
    /*0xe018*/ SDL_SCANCODE_COPY,
    /*0xe019*/ SDL_SCANCODE_MEDIA_NEXT_TRACK,
    /*0xe01a*/ SDL_SCANCODE_UNKNOWN,
    /*0xe01b*/ SDL_SCANCODE_UNKNOWN,
    /*0xe01c*/ SDL_SCANCODE_KP_ENTER,
    /*0xe01d*/ SDL_SCANCODE_RCTRL,
    /*0xe01e*/ SDL_SCANCODE_UNKNOWN,
    /*0xe01f*/ SDL_SCANCODE_MODE,
    /*0xe020*/ SDL_SCANCODE_MUTE,
    /*0xe021*/ SDL_SCANCODE_UNKNOWN, // LaunchApp2
    /*0xe022*/ SDL_SCANCODE_MEDIA_PLAY_PAUSE,
    /*0xe023*/ SDL_SCANCODE_UNKNOWN,
    /*0xe024*/ SDL_SCANCODE_MEDIA_STOP,
    /*0xe025*/ SDL_SCANCODE_UNKNOWN,
    /*0xe026*/ SDL_SCANCODE_UNKNOWN,
    /*0xe027*/ SDL_SCANCODE_UNKNOWN,
    /*0xe028*/ SDL_SCANCODE_UNKNOWN,
    /*0xe029*/ SDL_SCANCODE_UNKNOWN,
    /*0xe02a*/ SDL_SCANCODE_UNKNOWN,
    /*0xe02b*/ SDL_SCANCODE_UNKNOWN,
    /*0xe02c*/ SDL_SCANCODE_MEDIA_EJECT,
    /*0xe02d*/ SDL_SCANCODE_UNKNOWN,
    /*0xe02e*/ SDL_SCANCODE_VOLUMEDOWN,
    /*0xe02f*/ SDL_SCANCODE_UNKNOWN,
    /*0xe030*/ SDL_SCANCODE_VOLUMEUP,
    /*0xe031*/ SDL_SCANCODE_UNKNOWN,
    /*0xe032*/ SDL_SCANCODE_AC_HOME,
    /*0xe033*/ SDL_SCANCODE_UNKNOWN,
    /*0xe034*/ SDL_SCANCODE_UNKNOWN,
    /*0xe035*/ SDL_SCANCODE_KP_DIVIDE,
    /*0xe036*/ SDL_SCANCODE_RSHIFT,
    /*0xe037*/ SDL_SCANCODE_PRINTSCREEN,
    /*0xe038*/ SDL_SCANCODE_RALT,
    /*0xe039*/ SDL_SCANCODE_UNKNOWN,
    /*0xe03a*/ SDL_SCANCODE_UNKNOWN,
    /*0xe03b*/ SDL_SCANCODE_HELP,
    /*0xe03c*/ SDL_SCANCODE_UNKNOWN,
    /*0xe03d*/ SDL_SCANCODE_UNKNOWN,
    /*0xe03e*/ SDL_SCANCODE_UNKNOWN,
    /*0xe03f*/ SDL_SCANCODE_UNKNOWN,
    /*0xe040*/ SDL_SCANCODE_UNKNOWN,
    /*0xe041*/ SDL_SCANCODE_UNKNOWN,
    /*0xe042*/ SDL_SCANCODE_UNKNOWN,
    /*0xe043*/ SDL_SCANCODE_UNKNOWN,
    /*0xe044*/ SDL_SCANCODE_UNKNOWN,
    /*0xe045*/ SDL_SCANCODE_NUMLOCKCLEAR,
    /*0xe046*/ SDL_SCANCODE_PAUSE,
    /*0xe047*/ SDL_SCANCODE_HOME,
    /*0xe048*/ SDL_SCANCODE_UP,
    /*0xe049*/ SDL_SCANCODE_PAGEUP,
    /*0xe04a*/ SDL_SCANCODE_UNKNOWN,
    /*0xe04b*/ SDL_SCANCODE_LEFT,
    /*0xe04c*/ SDL_SCANCODE_UNKNOWN,
    /*0xe04d*/ SDL_SCANCODE_RIGHT,
    /*0xe04e*/ SDL_SCANCODE_UNKNOWN,
    /*0xe04f*/ SDL_SCANCODE_END,
    /*0xe050*/ SDL_SCANCODE_DOWN,
    /*0xe051*/ SDL_SCANCODE_PAGEDOWN,
    /*0xe052*/ SDL_SCANCODE_INSERT,
    /*0xe053*/ SDL_SCANCODE_DELETE,
    /*0xe054*/ SDL_SCANCODE_UNKNOWN,
    /*0xe055*/ SDL_SCANCODE_UNKNOWN,
    /*0xe056*/ SDL_SCANCODE_UNKNOWN,
    /*0xe057*/ SDL_SCANCODE_UNKNOWN,
    /*0xe058*/ SDL_SCANCODE_UNKNOWN,
    /*0xe059*/ SDL_SCANCODE_UNKNOWN,
    /*0xe05a*/ SDL_SCANCODE_UNKNOWN,
    /*0xe05b*/ SDL_SCANCODE_LGUI,
    /*0xe05c*/ SDL_SCANCODE_RGUI,
    /*0xe05d*/ SDL_SCANCODE_APPLICATION,
    /*0xe05e*/ SDL_SCANCODE_POWER,
    /*0xe05f*/ SDL_SCANCODE_SLEEP,
    /*0xe060*/ SDL_SCANCODE_UNKNOWN,
    /*0xe061*/ SDL_SCANCODE_UNKNOWN,
    /*0xe062*/ SDL_SCANCODE_UNKNOWN,
    /*0xe063*/ SDL_SCANCODE_UNKNOWN,
    /*0xe064*/ SDL_SCANCODE_UNKNOWN,
    /*0xe065*/ SDL_SCANCODE_AC_SEARCH,
    /*0xe066*/ SDL_SCANCODE_AC_BOOKMARKS,
    /*0xe067*/ SDL_SCANCODE_AC_REFRESH,
    /*0xe068*/ SDL_SCANCODE_AC_STOP,
    /*0xe069*/ SDL_SCANCODE_AC_FORWARD,
    /*0xe06a*/ SDL_SCANCODE_AC_BACK,
    /*0xe06b*/ SDL_SCANCODE_UNKNOWN,    // AL Local Machine Browser
    /*0xe06c*/ SDL_SCANCODE_UNKNOWN,    // AL Email Reader
    /*0xe06d*/ SDL_SCANCODE_MEDIA_SELECT,
    /*0xe06e*/ SDL_SCANCODE_UNKNOWN,
    /*0xe06f*/ SDL_SCANCODE_UNKNOWN,
    /*0xe070*/ SDL_SCANCODE_UNKNOWN,
    /*0xe071*/ SDL_SCANCODE_UNKNOWN,
    /*0xe072*/ SDL_SCANCODE_UNKNOWN,
    /*0xe073*/ SDL_SCANCODE_UNKNOWN,
    /*0xe074*/ SDL_SCANCODE_UNKNOWN,
    /*0xe075*/ SDL_SCANCODE_UNKNOWN,
    /*0xe076*/ SDL_SCANCODE_UNKNOWN,
    /*0xe077*/ SDL_SCANCODE_UNKNOWN,
    /*0xe078*/ SDL_SCANCODE_UNKNOWN,
    /*0xe079*/ SDL_SCANCODE_UNKNOWN,
    /*0xe07a*/ SDL_SCANCODE_UNKNOWN,
    /*0xe07b*/ SDL_SCANCODE_UNKNOWN,
    /*0xe07c*/ SDL_SCANCODE_UNKNOWN,
    /*0xe07d*/ SDL_SCANCODE_UNKNOWN,
    /*0xe07e*/ SDL_SCANCODE_UNKNOWN,
    /*0xe07f*/ SDL_SCANCODE_UNKNOWN
};

static WindowsDeviceManager *g_managerInstance = nullptr;
static std::atomic_bool g_winApiInit { false };
static std::atomic_bool g_winApiDeInit { false };

static std::atomic_uint_fast16_t g_keyModState { 0 };      // SDL_Keymod mapping
static std::atomic_uint_fast8_t g_mouseButtonState { 0 };  // bit 0 = button 1, bit 1 = btn 2, etc; equivalent to SDL_MouseButtonFlags
static uint8_t g_kbState[256] { 0 };  // Windows keyboard state, not SDL
static std::atomic_bool g_keystateLock { false };

static const constexpr uint16_t KEY_STATE_DOWN = 0x80;
static const constexpr uint16_t KEY_STATE_TOGGLE = 0x01;
static const constexpr uint16_t KMOD_TOGGLE_MASK = 0xF000;

struct ModKeyIds {
	SDL_Keymod kmod;
	SDL_Scancode sc;
	uint32_t vk;
};

static const constexpr ModKeyIds g_toggleKeys[] = {
  { SDL_KMOD_NUM,    SDL_SCANCODE_NUMLOCKCLEAR, VK_NUMLOCK },
  { SDL_KMOD_CAPS,   SDL_SCANCODE_CAPSLOCK,     VK_CAPITAL },
  { SDL_KMOD_SCROLL, SDL_SCANCODE_SCROLLLOCK,   VK_SCROLL },
  // { SDL_KMOD_MODE,   SDL_SCANCODE_MODE,         VK_MODECHANGE },
};

using SdlKeymodMap = QHash<uint32_t, SDL_Keymod>;
Q_GLOBAL_STATIC_WITH_ARGS(const SdlKeymodMap, g_vkKeyToSdlMod, ({
	{ VK_LSHIFT,     SDL_KMOD_LSHIFT },
	{ VK_RSHIFT,     SDL_KMOD_RSHIFT },
	//{ VK_,         SDL_KMOD_LEVEL5 },
	{ VK_LCONTROL,   SDL_KMOD_LCTRL  },
	{ VK_RCONTROL,   SDL_KMOD_RCTRL  },
	{ VK_LMENU,      SDL_KMOD_LALT   },
	{ VK_RMENU,      SDL_KMOD_RALT   },
	{ VK_LWIN,       SDL_KMOD_LGUI   },
	{ VK_RWIN,       SDL_KMOD_RGUI   },
	{ VK_NUMLOCK,    SDL_KMOD_NUM    },
	{ VK_CAPITAL,    SDL_KMOD_CAPS   },
	{ VK_MODECHANGE, SDL_KMOD_MODE   },
	{ VK_SCROLL,     SDL_KMOD_SCROLL },
	{ VK_CONTROL,    SDL_KMOD_CTRL   },
	{ VK_SHIFT,      SDL_KMOD_SHIFT  },
	{ VK_MENU,       SDL_KMOD_ALT    },
	//{ VK_,         SDL_KMOD_GUI    },
}))

static void toggleKbdModState(/*SDL_Keymod &target,*/ SDL_Keymod mod, bool on, uint8_t vk) {
	if (on)
		g_keyModState |= mod;
	else
		g_keyModState &= ~mod;

	g_kbState[vk] = on ? ((mod & KMOD_TOGGLE_MASK) ? KEY_STATE_TOGGLE : KEY_STATE_DOWN) : 0x00;
}

static LRESULT CALLBACK windowsLLHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0 || nCode != HC_ACTION || !g_managerInstance)
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	uint8_t btn = 0;
	bool flag = false;
	switch (wParam)
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			const PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
			QMetaObject::invokeMethod(g_managerInstance, &WindowsDeviceManager::keyEventHandler, Qt::QueuedConnection, p->vkCode, p->scanCode, p->flags);
			// qCDebug(lcNAPI) << QThread::currentThread();
			break;
		}

		// Mouse events

		case WM_MOUSEMOVE: {
			const PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT)lParam;
			QMetaObject::invokeMethod(g_managerInstance, &WindowsDeviceManager::mouseEventHandler, Qt::QueuedConnection, p->time, p->pt.x, p->pt.y, 0, false, 0);
			break;
		}

		case WM_MOUSEHWHEEL:   // horizontal
			flag = true;
			Q_FALLTHROUGH();
		case WM_MOUSEWHEEL: {  // vertical
			const PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT)lParam;
			// reduce scroll amount by the value of one "click," which have different constants for H and V scroll; vertical scroll direction needs to be reversed
			const int16_t delta = (GET_WHEEL_DELTA_WPARAM(p->mouseData) / (flag ? 0x3B88 : (WHEEL_DELTA * -1)));
			// qCDebug(lcNAPI).nospace() << "WM_MOUSE[H]WHEEL: " << "wParam:" << LOG_HEX4(wParam) << DBG_SEP << p->pt.x << DBG_SEP << p->pt.y << DBG_SEP << "mouseData:" << LOG_HEX4(GET_WHEEL_DELTA_WPARAM(p->mouseData));
			QMetaObject::invokeMethod(g_managerInstance, &WindowsDeviceManager::mouseEventHandler, Qt::QueuedConnection, p->time, p->pt.x, p->pt.y, 0, flag, delta);
			break;
		}

		// down
		case WM_LBUTTONDOWN:
			btn = MouseButton::MB_Left;
			Q_FALLTHROUGH();
		case WM_RBUTTONDOWN:
			if (!btn) btn = MouseButton::MB_Right;
			Q_FALLTHROUGH();
		case WM_MBUTTONDOWN:
			if (!btn) btn = MouseButton::MB_Middle;
			Q_FALLTHROUGH();
		case WM_XBUTTONDOWN:
			if (!btn) btn = 255;  // magic value to avoid setting it in fallthrough
			flag = true;
			Q_FALLTHROUGH();
		// up
		case WM_LBUTTONUP:
			if (!btn) btn = MouseButton::MB_Left;
			Q_FALLTHROUGH();
		case WM_RBUTTONUP:
			if (!btn) btn = MouseButton::MB_Right;
			Q_FALLTHROUGH();
		case WM_MBUTTONUP:
			if (!btn) btn = MouseButton::MB_Middle;
			Q_FALLTHROUGH();
		case WM_XBUTTONUP:
		{
			const PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT)lParam;
			if (!btn || btn == 255)
				btn = GET_XBUTTON_WPARAM(p->mouseData) + 3;
			QMetaObject::invokeMethod(g_managerInstance, &WindowsDeviceManager::mouseEventHandler, Qt::QueuedConnection, p->time, p->pt.x, p->pt.y, btn, flag, 0);
			break;
		}

		default:
			qCDebug(lcNAPI).nospace() << "Windows hook nCode: " << LOG_HEX4(nCode) << DBG_SEP << "wParam:" << LOG_HEX8(wParam) << DBG_SEP << "lParam:" << LOG_HEX8(lParam);
			break;
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}


//
// WindowsHookWorker
//

class WindowsHookWorker : public QObject
{
		Q_OBJECT
	public:
		using QObject::QObject;

		~WindowsHookWorker() {
			enableKeyboardHook(false);
			enableMouseHook(false);
		}

		bool isEnabled() const {
			QReadLocker lk(&m_hookMtx);
			return (hkLowLevelKybd != NULL || hkLowLevelMous != NULL);
		}

		bool isEnabled(const QByteArray &uid) const
		{
			QReadLocker lk(&m_hookMtx);
			if (uid == DI_SYSTEM_KEYBOARD_UID)
				return hkLowLevelKybd != NULL;
			if (uid == DI_SYSTEM_MOUSE_UID)
				return hkLowLevelMous != NULL;
			return false;
		}

	public Q_SLOTS:
		void hook(const QByteArray &uid, bool en)
		{
			if (uid == DI_SYSTEM_KEYBOARD_UID)
				enableKeyboardHook(en);
			else if (uid == DI_SYSTEM_MOUSE_UID)
				enableMouseHook(en);
		}

		void enableKeyboardHook(bool en)
		{
			QWriteLocker lk(&m_hookMtx);

			if (en && hkLowLevelKybd == NULL) {
				hkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, windowsLLHook, NULL, NULL);
				if (hkLowLevelKybd == NULL) {
					en = false;
				}
				else {
					// Get initial toggle key states
					for (const auto &key : g_toggleKeys)
						toggleKbdModState(key.kmod, (GetKeyState(key.vk) & KEY_STATE_TOGGLE), key.vk);
				}
			}
			else if (!en && hkLowLevelKybd != NULL) {
				UnhookWindowsHookEx(hkLowLevelKybd);
				hkLowLevelKybd = NULL;
			}
			else {
				return;
			}

			Q_EMIT hooked(DI_SYSTEM_KEYBOARD_UID, en);
		}

		void enableMouseHook(bool en)
		{
			QWriteLocker lk(&m_hookMtx);

			if (en && hkLowLevelMous == NULL) {
				hkLowLevelMous = SetWindowsHookEx(WH_MOUSE_LL, windowsLLHook, NULL, NULL);
				if (hkLowLevelMous == NULL)
					en = false;
			}
			else if (!en && hkLowLevelMous != NULL) {
				UnhookWindowsHookEx(hkLowLevelMous);
				hkLowLevelMous = NULL;
			}
			else {
				return;
			}

			Q_EMIT hooked(DI_SYSTEM_MOUSE_UID, en);
		}

	Q_SIGNALS:
		void hooked(const QByteArray &uid, bool hooked);

	private:
		HHOOK hkLowLevelKybd = NULL;
		HHOOK hkLowLevelMous = NULL;
		mutable QReadWriteLock m_hookMtx;
};


//
// WindowsDeviceManager
//

WindowsDeviceManager::WindowsDeviceManager(QObject *parent) :
	IApiManager{parent},
	m_hookWorker{new WindowsHookWorker()},
  m_workerThread{new QThread()}
{
	if (g_managerInstance)
		delete g_managerInstance;

	g_keyModState = 0;
	g_mouseButtonState = 0;
	g_keystateLock = false;
	SDL_zeroa(g_kbState);

	connect(this, &WindowsDeviceManager::enableDeviceHook, m_hookWorker, &WindowsHookWorker::hook, Qt::QueuedConnection);
	// connect(m_hookWorker, &WindowsHookWorker::hooked, this, &WindowsDeviceManager::deviceReportToggled, Qt::QueuedConnection);
	connect(m_hookWorker, &WindowsHookWorker::hooked, this, &WindowsDeviceManager::onDeviceHooked, Qt::QueuedConnection);
	m_hookWorker->moveToThread(m_workerThread);
	m_workerThread->setObjectName("WindowsHookWorkerThread");
	// m_workerThread->start();

	g_managerInstance = this;
}

WindowsDeviceManager::~WindowsDeviceManager()
{
	g_managerInstance = nullptr;  // prevent any further message handling in hook(s)

	deinit();
	delete m_hookWorker;
	delete m_workerThread;
}

bool WindowsDeviceManager::init()
{
	if (g_winApiInit)
		return true;
	// SDL_SetHint(SDL_HINT_KEYCODE_OPTIONS, "hide_numpad");

	SDL_SetScancodeName(SDL_SCANCODE_APPLICATION, "Application");  // SDL sets this to "Menu" on Windows
	// SDL_SetScancodeName(SDL_SCANCODE_LGUI, "Left Windows");
	// SDL_SetScancodeName(SDL_SCANCODE_RGUI, "Right Windows");

	g_winApiInit = true;
	return true;
}

void WindowsDeviceManager::deinit()
{
	if (!g_winApiInit || g_winApiDeInit)
		return;
	g_winApiDeInit = true;
	disconnectDevice(DI_SYSTEM_KEYBOARD_UID);
	disconnectDevice(DI_SYSTEM_MOUSE_UID);
	Q_EMIT deviceRemoved(DI_SYSTEM_KEYBOARD_UID);
	Q_EMIT deviceRemoved(DI_SYSTEM_MOUSE_UID);
	if (m_workerThread->isRunning()) {
		m_workerThread->quit();
		m_workerThread->wait();
	}
	g_winApiInit = false;
	g_winApiDeInit = false;
}

void WindowsDeviceManager::scanDevices()
{
	// if (SDL_HasKeyboard()) {
	// SDL VIDEO must be initialized for it to detect a keyboard, so assume for now we have one...
	DeviceDescriptor kdd { DeviceAPI::DA_NATIVE, DeviceType::DT_Keyboard, DI_SYSTEM_KEYBOARD_UID, DI_SYSTEM_KEYBOARD_ID, 1, Devices::tr("Keyboard") };
	Q_EMIT deviceDiscovered(kdd);
	// }

	if (GetSystemMetrics(SM_MOUSEPRESENT)) {
		DeviceDescriptor mdd { DeviceAPI::DA_NATIVE, DeviceType::DT_MouseType, DI_SYSTEM_MOUSE_UID, DI_SYSTEM_MOUSE_ID, 1, Devices::tr("Mouse") };
		Q_EMIT deviceDiscovered(mdd);
	}
	// discoverScreens();
}

void WindowsDeviceManager::connectDevice(const QByteArray &uid)
{
	if ((uid != DI_SYSTEM_KEYBOARD_UID && uid != DI_SYSTEM_MOUSE_UID) || !m_hookWorker || m_hookWorker->isEnabled(uid))
		return;
	if (!m_workerThread->isRunning())
		m_workerThread->start();
	Q_EMIT enableDeviceHook(uid, true);
}

void WindowsDeviceManager::disconnectDevice(const QByteArray &uid)
{
	if (m_hookWorker && m_hookWorker->isEnabled(uid))
		Q_EMIT enableDeviceHook(uid, false);
}

void WindowsDeviceManager::onDeviceHooked(const QByteArray &uid, bool started)
{
	Q_EMIT deviceReportToggled(uid, started);

	if (!started && !m_hookWorker->isEnabled())
		m_workerThread->quit();
}

void WindowsDeviceManager::sendDeviceReport(const QByteArray &uid)
{
	if (!m_hookWorker || !m_hookWorker->isEnabled(uid))
		return;

	const uint64_t ts = SDL_GetTicksNS();

	if (uid == DI_SYSTEM_KEYBOARD_UID) {
		const uint16_t mods = g_keyModState;

		for (const auto &key : g_toggleKeys) {
			// toggleKbdModState(key.kmod, (GetKeyState(key.vk) & KEY_STATE_TOGGLE), key.vk);
			Q_EMIT deviceEvent(new DeviceKeyEvent(
				ts, (uint)key.sc, mods,
				SDL_GetKeyFromScancode(key.sc, mods, false),
				key.vk, 0, (mods & key.kmod), false, SDL_GetScancodeName(key.sc)
			));
		}
		return;
	}

	if (uid == DI_SYSTEM_MOUSE_UID) {
		const auto dt = DeviceTypes(DeviceType::DT_Pointer | DeviceType::DT_Mouse);
		Q_EMIT deviceEvent(new DeviceMotionEvent(ts, dt, DI_SYSTEM_MOUSE_UID, 1));

		const uint8_t btnState = g_mouseButtonState;
		int count = std::min(GetSystemMetrics(SM_CMOUSEBUTTONS), 5);
		// qCDebug(lcNAPI) << "System shows" << count << "mouse buttons.";
		for (int i=0; i < count; ++i)
			Q_EMIT deviceEvent(new DeviceButtonEvent(ts, dt, DI_SYSTEM_MOUSE_UID, i+1, (btnState & (1 << i))));

		count = GetSystemMetrics(SM_MOUSEWHEELPRESENT);
		if (count)
			Q_EMIT deviceEvent(new DeviceScrollEvent(ts, dt, DI_SYSTEM_MOUSE_UID, 1));

	}
}

void WindowsDeviceManager::keyEventHandler(uint32_t vkCode, uint32_t scanCode, uint32_t flags)
{
	if (!vkCode || vkCode >= 0xFF)
		return;

	SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
	SDL_Keycode key = SDLK_UNKNOWN;
	const bool down = !(flags & LLKHF_UP);
	const bool repeat = down && (g_kbState[vkCode] & KEY_STATE_DOWN); //(GetAsyncKeyState(p->vkCode) & (1 << 15));
	bool skipEvent = false;

	if (!repeat) {
		if (g_keystateLock)
			return;
		g_keystateLock = true;

		if (const auto mod = g_vkKeyToSdlMod->value(vkCode, SDL_KMOD_NONE); mod != SDL_KMOD_NONE) {
			// handle toggle modifiers (caps/num/scroll/mode lock) differently...
			if (mod & KMOD_TOGGLE_MASK) {
				// skip sending key event if the mod is already toggled on
				skipEvent = (g_keyModState & mod) > 0;
				// only set them on key down and toggle based on current state
				if (down)
					g_keyModState ^= mod;
				g_kbState[vkCode] = ((g_keyModState & mod) ? KEY_STATE_TOGGLE : 0x00) | (down ? KEY_STATE_DOWN : 0x00);
			}
			else {
				toggleKbdModState(mod, down, vkCode);
				// Win api ToUnicode() needs the "generic" modifier key states for proper key text lookup
				const uint16_t mods = g_keyModState;
				g_kbState[VK_SHIFT]   = (mods & SDL_KMOD_SHIFT) ? KEY_STATE_DOWN : 0x00;
				g_kbState[VK_CONTROL] = (mods & SDL_KMOD_CTRL)  ? KEY_STATE_DOWN : 0x00;
				g_kbState[VK_MENU]    = (mods & SDL_KMOD_ALT)   ? KEY_STATE_DOWN : 0x00;
			}
			// SDL_SetModState(mods);
		}
		else {
			g_kbState[vkCode] = down ? KEY_STATE_DOWN : 0x00;
		}
		g_keystateLock = false;
	}

	if (skipEvent)
		return;

	const uint16_t mods = g_keyModState; // SDL_Keymod
	QString keyName;
	QString keyText;
	wchar_t nameBuff[16] {0};

	if (!scanCode) {
		// Media keys may not send a scan code with the actual key event for some reason, but a lookup works.
		scanCode = MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC_EX);
		// qCDebug(lcNAPI).nospace() << LOG_HEX4(vkCode) << DBG_SEP << LOG_HEX4(scanCode) << DBG_SEP << LOG_HEX4(flags);
	}

	uint sci = scanCode & 0xFF;  // index into windows_scancode_table array
	if (flags & LLKHF_EXTENDED)
		sci += 0x80;

	if (sci < SDL_arraysize(windows_scancode_table))
		scancode = windows_scancode_table[sci];
	else
		scancode = SDL_SCANCODE_UNKNOWN;

	if (scancode != SDL_SCANCODE_UNKNOWN) {
		// key = SDL_GetKeyFromScancode(scancode, mod, false);
		key = SDL_GetKeyFromScancode(scancode, (SDL_Keymod)mods, true);
		keyName = SDL_GetScancodeName(scancode); // SDL_GetKeyName(key);
	}
	else {
		// fallback, shouldn't happen.
		scancode = (SDL_Scancode)scanCode;
		if (const int res = GetKeyNameTextW(((scanCode & 0xFF) << 16) | ((flags & LLKHF_EXTENDED) << 24), nameBuff, SDL_arraysize(nameBuff)); res > 0) {
			keyName = QString::fromWCharArray(nameBuff, res);
			key = SDL_GetKeyFromName(keyName.toUtf8().constData());
		}
	}

	if (const int res = ToUnicode(vkCode, scanCode, g_kbState, nameBuff, SDL_arraysize(nameBuff), 0x04); res > 0)
		keyText = QString::fromWCharArray(nameBuff, res);

	DeviceKeyEvent *ev = new DeviceKeyEvent(SDL_GetTicksNS(), (uint)scancode, mods, (uint)key, vkCode, scanCode, down, repeat, keyName, keyText);

#if 0
	if (down) {
		// const auto eventKeyCode = SDL_GetKeyFromScancode(scancode, mods, false);
		qCDebug(lcNAPI).nospace() << "Got Keyboard event:\n" << *ev << '\n'
		                             // << " Alternate key,name: "<< LOG_HEX4(eventKeyCode) << DBG_SEP << QString(SDL_GetKeyName(eventKeyCode)) << '\n'
		                             // << " NTV scn,key,flags,extra: " << LOG_HEX4(scanCode) << DBG_SEP << LOG_HEX4(vkCode) << DBG_SEP << LOG_HEX4(flags)  // << DBG_SEP << LOG_HEX4(p->dwExtraInfo)
		;
		// std::wcout << "  NAME: " << quoted(keyName.toStdWString()) << " TEXT: " << quoted(keyText.toStdWString()) << std::endl;
	}
	// qCDebug(lcNAPI) << QByteArray((const char *)g_kbState, 256).toHex(':');
#endif

	Q_EMIT deviceEvent(ev);
}

void WindowsDeviceManager::mouseEventHandler(uint32_t time, long ptX, long ptY, uint8_t button, bool pressOrWheelH, int16_t wheel)
{
	DeviceEvent *ev = nullptr;
	if (!!wheel) {
		ev = new DeviceScrollEvent(time, 1, ptX, ptY, (pressOrWheelH ? wheel : 0), (pressOrWheelH ? 0 : wheel));
	}
	else if (!!button) {
		ev = new DeviceButtonEvent(time, button, pressOrWheelH, ptX, ptY);
		if (button < 9) {
			if (pressOrWheelH)
				g_mouseButtonState |= (1 << (button - 1));
			else
				g_mouseButtonState &= ~(1 << (button - 1));
		}
	}
	else {
		long rx{0}, ry{0};
		// try get relative movement with Win API; need to get at least 2 points from the history because the first one returned is always the current position
		MOUSEMOVEPOINT mpIn { ptX, ptY, time, 0 };
		MOUSEMOVEPOINT mpOut[2];
		const int pts = GetMouseMovePointsEx(sizeof(MOUSEMOVEPOINT), &mpIn, mpOut, 2, GMMP_USE_DISPLAY_POINTS);
		if (pts > 1) {
			// need to normalize the returned point values, as per GetMouseMovePointsEx() docs
			if (mpOut[1].x > 32767)
				mpOut[1].x -= 65536;
			if (mpOut[1].y > 32767)
				mpOut[1].y -= 65536;
			// calc the deltas; positive values are to the right and down (east and south)
			rx = ptX - mpOut[1].x;
			ry = ptY - mpOut[1].y;
			// qCDebug(lcNAPI) << "GetMouseMovePoints() returned:" << mpOut[0].x << mpOut[0].y << mpOut[1].x << mpOut[1].y << "result:" << rx << ry;
		}
		// else if (pts < 0)
		// 	qCDebug(lcNAPI) << "GetMouseMovePoints() returned error:" << GetLastError();
		// else
		// 	qCDebug(lcNAPI) << "GetMouseMovePoints() returned:" << pts << "points";
		// const uint8_t btnState = g_mouseButtonState;
		ev = new DeviceMotionEvent(time, 1, ptX, ptY, rx, ry/*, btnState*/);
	}

	if (ev) {
		ev->deviceUid = DI_SYSTEM_MOUSE_UID;
		ev->deviceType = DeviceType::DT_MouseType;
		Q_EMIT deviceEvent(ev);
	}
}

#if 0
using ScreenInfoList = QList<ScreenInfo>;
Q_GLOBAL_STATIC(ScreenInfoList, g_screenInfoList)

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC /*hdcMonitor*/, LPRECT pRect, LPARAM numScreens)
{
	const int width = pRect->right - pRect->left;
	const int height = pRect->bottom - pRect->top;
	if (width <= 0 || height <= 0)
		return TRUE;

	bool primary = false;
	MONITORINFOEX mi;
	mi.cbSize = sizeof(MONITORINFOEX);
	if (GetMonitorInfoW(hMon, &mi)) {
		primary = (mi.dwFlags & MONITORINFOF_PRIMARY);
		// si.name.assign(mi.szDevice);
	}

	QList<ScreenInfo> *list = g_screenInfoList; //(QList<ScreenInfo> *)context;

	ScreenInfo si { (short)(list->size() + 1), primary, false, (uint)width, (uint)height, pRect->left, pRect->top };

	qCDebug(lcNAPI) << "Got monitor info:" << si.index << "out of" << (int)numScreens << "Pos:" << si.posX << si.posY << "Size:" << si.width << si.height << "Primary?" << si.isPrimary << "Name;" << si.name;

	list->append(std::move(si));

	if ((int)numScreens == (int)si.index) {
		qCDebug(lcNAPI) << "Finished monitor enumeration";
		// return FALSE;
	}

	return TRUE;
}

void WindowsDeviceManager::discoverScreens()
{
	g_screenInfoList->clear();
	const int numScreens = GetSystemMetrics(SM_CMONITORS);

	if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)numScreens)) {
		qCWarning(lcNAPI) << "EnumDisplayMonitors return false" << GetLastError();
	}
	qCDebug(lcNAPI) << "Desktop dims:" << GetSystemMetrics(SM_XVIRTUALSCREEN) << GetSystemMetrics(SM_YVIRTUALSCREEN) << GetSystemMetrics(SM_CXVIRTUALSCREEN) << GetSystemMetrics(SM_CYVIRTUALSCREEN);
}
#endif

#include "WindowsDeviceManager.moc"
#include "moc_WindowsDeviceManager.cpp"
