# Device Input Plugin for Touch Portal

[![Made for Touch Portal](https://img.shields.io/static/v1?style=flat&labelColor=5884b3&color=black&label=made%20for&message=Touch%20Portal&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAYAAAAfSC3RAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAetJREFUeNp0UruqWlEQXUePb1HERi18gShYWVqJYGeXgF+Qzh9IGh8QiOmECIYkpRY21pZWFnZaqWBhUG4KjWih4msys8FLbrhZMOfsx6w1e9beWjAYBOMtx0eOGBEZzuczrtcreAyTyQSz2QxN04j3f3J84vim8+cNR4s3rKfTSUQQi8UQjUYlGYvFAtPpVIQ0u90eZrGvnHLXuOKcB1GpkkqlUCqVEA6HsVqt4HA4EAgEMJvNUC6XMRwOwWTRfhIi3e93WK1W1Go1dbTBYIDj8YhOp4NIJIJGo4FEIoF8Po/JZAKLxQIIUSIUChGrEy9Sr9cjQTKZJJvNRtlsVs3r9Tq53W6Vb+Cy0rQyQtd1OJ1O9b/dbpCTyHoul1O9z+dzGI1Gla7jFUiyGBWPx9FsNpHJZNBqtdDtdlXfAv3vZLmCB6SiJIlJhUIB/X7/cS0viXI8n8+nrBcRIblcLlSrVez3e4jrD6LsK3O8Xi8Vi0ViJ4nVid2kB3a7HY3HY2q325ROp8nv94s5d0XkSsR90OFwoOVySaPRiF6DiHs8nmdXn+QInIxKpaJclWe4Xq9fxGazAQvDYBAKfssDeMeD7zITc1gR/4M8isvlIn2+F3N+cIjMB76j4Ha7fb7bf8H7v5j0hYef/wgwAKl+FUPYXaLjAAAAAElFTkSuQmCC)](https://www.touch-portal.com)
[![Latest Release](https://img.shields.io/github/v/release/mpaperno/DeviceInput-TouchPortal-Plugin?include_prereleases&display_name=release)](https://github.com/mpaperno/DeviceInput-TouchPortal-Plugin/releases)
![Supported Platforms](https://img.shields.io/badge/platforms-Windows%20|%20MacOS%20|%20Linux-AA7722)
[![GPLv3 License](https://img.shields.io/badge/license-GPLv3-blue.svg)](LICENSE.GPL.txt)
[![Discord](https://img.shields.io/static/v1?style=flat&color=7289DA&&labelColor=7289DA&message=Discord%20Chat&label=&logo=discord&logoColor=white)](https://discord.gg/AhVYXRQTHu)
<!-- <img alt="Lines of code" src="https://img.shields.io/tokei/lines/github/mpaperno/DeviceInput-TouchPortal-Plugin?color=green&label=LoC&logo=cplusplus&logoColor=f34b7d"> -->


**Hardware device input plugin for [Touch Portal](https://www.touch-portal.com/) macro launcher software.**
**Use game controllers, keyboard, or mouse, to trigger Touch Portal actions.**

----------

## Features
**This plugin allows using external hardware devices, such as game controllers, keyboards, etc, as inputs for Touch Portal.**<br />

* Trigger Touch Portal actions based on device button or key presses, axis movement, wheel/scroll inputs, and more.
* Connect on-demand to monitor any supported device type detected on the system the plugin is running on.
* Input reports are delivered as Touch Portal _States_ with single values and as _Events_ with attached structured data.
* Device monitoring is passive, meaning the devices can still be used with other software at the same time.
* Can run on a remote system with a network connection to a Touch Portal desktop instance.
* Runs on **Windows, Linux, and MacOS** (including Linux ARM devices such as Raspberry Pi, and an Android version is also possible).

### Currently supported device types

* Game Controllers, such as joysticks, gamepads, wheels, etc.
* Keyboard  (Windows only for now)
* Mouse / Pointing device  (Windows only for now)

### Possible future device support

* Keyboard/Mouse support on Linux (maybe Mac)
* Touch
* Sensors (accelerometer/gyro)
* Generic HID reporting


-------------
## Download and Install

Note: As with all plugins, this requires the Touch Portal Pro (paid) version. TP v4+ is required. Use the latest available Touch Portal version for best results.

1. Get the latest version of this plugin for your operating system from the [Releases](https://github.com/mpaperno/DeviceInput-TouchPortal-Plugin/releases) page.
2. The plugin is distributed and installed as a standard Touch Portal `.tpp` plugin file. If you know how to import a plugin, just do that... otherwise continue here.
3. Import the plugin:
    1. Start/open _Touch Portal_.
    2. Click the "Quick Actions" icon at the top-right in the title bar (next to the "minimize" button) and select "Import plug-in..." from the menu.
    3. Browse to where you downloaded this plugin's `.tpp` file and select it.
    4. When prompted by _Touch Portal_ to trust the plugin startup script, select "Trust Always" or "Yes" (the source code is public!).
       * "Trust Always" will automatically start the plugin each time Touch Portal starts.
       * "Yes" will start the plugin this time and then prompt again each time Touch Portal starts.
       * If you select "No" then you can still start the plugin manually from Touch Portal's _Settings -> Plug-ins_ dialog.
4. That's it. You should now have the plugin's Actions, Events, and States available to you in Touch Portal.

### Updates

Unless stated otherwise in the notes of a particular release version, it is OK to just re-install a newer version of the plugin "on top of"
a previous version without un-installing the old version first. Either way is OK, just keep in mind that un-installing the plugin via Touch Portal
will also remove all plugin log files as well.

### Update Notifications

The latest version of this software is always published on the GitHub [Releases](https://github.com/mpaperno/TJoy/releases) page.

You have several options for getting **automatically notified** about new releases:

* In GitHub (with an account) you can _Watch_ -> _Custom_ -> _Releases_ this repository (button at top right).
* Subscribe to the [ATOM feed](https://github.com/mpaperno/DeviceInput-TouchPortal-Plugin/releases.atom) for release notifications.
* If you use **Discord**, subscribe to notifications on my server channel [#device-input](https://discord.gg/pN6wRrqwV8).


-------------
## Support and Discussion

Use the GitHub [Issues](https://github.com/mpaperno/DeviceInput-TouchPortal-Plugin/issues) feature for bug reports and concise feature suggestions.
Use [Discussions](https://github.com/mpaperno/DeviceInput-TouchPortal-Plugin/discussions) for any other topic.

There are also dedicated **Discord** channels on my server @ [#device-input-general](https://discord.gg/AhVYXRQTHu),
and at Touch Portal's Discord server @ [#device-input](https://discord.gg/FhYsZNFgyw).

**Your feedback, suggestions, and other input is always welcome!**


-------------
## Troubleshooting (Log File)

The plugin keeps a log file while running. This log file is in the plugin's installation folder, which will be in the Touch Portal data directory:<br/>

* Windows: `C:\Users\<User_Name>\AppData\Roaming\TouchPortal\plugins\DeviceInputPlugin\logs`
* Mac: `~/Documents/TouchPortal/plugins/DeviceInputPlugin/logs`
* Linux: `~/.config/TouchPortal/plugins/DeviceInputPlugin/logs`

By default all warnings and errors will be logged, as well as some basic information about the Touch Portal connection and the devices being used (if any).
This log is the first place to look if you suspect something isn't working correctly.

**This logged information will be vital in trying to track down any issues with the plugin code or functionality.** Please locate and consult your log file
before seeking support with the plugin, since I (or others) will very likely request to see it. The logs should not contain any sensitive or personal information,
(although it's always good to check before posting anything online ;-) ).

The log files are automatically rotated every day, and by default only the last 7 days are kept and the older ones deleted.


-------------
## Credits

This project is written, tested, and documented by myself, Maxim (Max) Paperno.<br/>
https://github.com/mpaperno/

**Contributions are welcome!**

Uses portions of the [_Qt Library_](http://qt.io) under the terms of the GPL v3 license.

Uses the [Simple DirectMedia Layer (SDL) v3](https://www.libsdl.org/) library
under the terms of the [zlib license](https://www.libsdl.org/license.php).


-------------
## Copyright, License, and Disclaimer

Device Input Plugin Project <br />
COPYRIGHT: Maxim Paperno; All Rights Reserved.

This program and associated files may be used under the terms of the GNU
General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License is included in this repository
and is also available at <http://www.gnu.org/licenses/>.

This project may also use 3rd-party Open Source software under the terms
of their respective licenses. The copyright notice above does not apply
to any 3rd-party components used within.


-------------
# Documentation

## Settings

* `Send Device Reports as States` - When enabled, each device event creates & updates a Touch Portal plugin State, with a value for the active control
  (axis, button, key, etc). For greater efficiency, these states can be disabled, for example if only using the (more flexible) Events system to handle
  input device updates.
* `Send Device Reports as Events` - When enabled, each device event sends a corresponding Touch Portal plugin Event, with local state values relevant
  to the event type. For greater efficiency, these events can be disabled, for example if only using the plugin States system to handle input device
  updates.

## Actions

* `Plugin Control Actions`
  * `Rescan System Devices` - Runs a full system scan for connected devices (of supported types), reporting any new, changed, or removed devices.
  * `Update All States & Events` - Re-sends all States and Events related to the plugin's status and reporting devices, such as plugin running state,
  list of connected devices, which devices have reporting enabled, and refreshes any active device reports.<br>
  This is especially useful when navigating to a page using these states/events, for example to refresh visual status indicators.
  * `Send System Displays Report` - Creates or updates dynamic plugin States with information about currently connected displays (screens/monitors) and overall desktop geometry.
    This information can be useful if tracking mouse/pointer movement to determine which monitor or part of the desktop the pointer is currently on.
* `Device Control Actions` - Controls the reporting state of connected devices.<br/>
  The action requires selecting a device from a list, and then the action to perform on that device.
  The list of devices is refreshed by the plugin based on currently connected devices.<br/>
  In addition to the actual devices found on the system, there are several "generic" choices added here for selecting devices, such as "All With Active Reports"
  and "First" and "Default" devices for common controller types (for example "First Gamepad" or "Default Joystick").
  "First" means the first connected device of a type as reported by the system, whereas "Default" devices can be specified by using the
  _Set a Default Device For Type_ action (described below).
  * `Toggle`, `Start` or `Stop` reporting for the chosen device.
  * `Refresh Report` to manually request an input report update for the chosen device.
  * `Clear Report Filter` clear any reporting filter set for the chosen device (see next action for details on filters).
* `Set Device Report Filter` - Set filter(s) for device reports. Filters enable limiting the amount of data sent to Touch Portal
  by only reporting the controls (buttons/axes/keys/etc) on that device that you're actually interested in.<br/>
  For example a joystick may have many buttons and several axes, but for usage in Touch Portal you may only need to know about a few particular button states.
  A filter can be set to exclude everything but the buttons you're interested in and, especially, eliminate all axis value reporting (which can generate a lot
  of data if the joystick is also being used for other things, like playing a game).
  * The filter format is described below in the [Report Filter Format](#report-filter-format) section.
  * To clear a report filter (and report all device events), specify an empty filter value or use the
    _Device Control Actions -> Clear Report Filter_ action described above.
* `Set a Default Device For Type` - This actions lets you specify a device which can be used as the "default" device for a particular type (eg. Joystick or Gamepad).
  Specifying defaults can simplify other device-specific actions (above) by only naming the actual device once, in this action,
  and then using the "Default" selection for that device type when setting up other actions or events.


## States

### Plugin Status States

* `Plugin running state (Stopped/Starting/Started)` - The plugin will update this value whenever it starts and stops, including at Touch Portal startup.<br/>
  The corresponding Event is [Plugin State Changed](#plugin-state-changed).
* `Name of Page currently active on TP device` - Just a convenience State for detecting when a page has been loaded (eg. to refresh data, start reporting, etc).<br/>
  The corresponding Event is [Current Page Changed](#current-page-changed).

### Device Information States

* `List of detected input device names` - A list of all currently detected (and supported) devices attached to the system.
  Names are separated with newline (`\n`) characters.
* `List of devices with active reports` - A list of devices for which input reporting is currently enabled.
  Names are separated with newline (`\n`) characters.
* `The most recently discovered input device` - Updated when a new device is connected to the system, either automatically detected or as a result of a manual scan.
* `The most recently removed input device` - Updated when a device is disconnected from the system, either automatically detected or as a result of a manual scan.
* `Name of most recent device for which reporting started` - Updated when input reporting starts for a device, eg. as a result of the _Start Reporting_ device control action being used.
* `Name of most recent device for which reporting stopped` - Updated when input reporting stops for a device, either manually initiated or if the device gets disconnected or when plugin shuts down.

### Device Assignment States

These states reflect which controller devices are currently considered "First" and "Default" for a particular type (Controller/Gamepad/Joystick/Throttle).<br/>
"First" means the first connected device of a type as reported by the system, whereas "Default" devices can be specified by using the _Set a Default Device For Type_ action (described above). The values of these states may be empty if no devices of that type are detected or no defaults are set.

### Dynamic States For Device Input Reports

When reporting for a device is enabled, the plugin will create and send a new _State_ for each detected input change.
For example if the first button on a monitored joystick is first pressed, a new _State_ will be created specific to that
device and that button. Next time the button is pressed (or released), that same _State_ is updated with the new button's value (pressed or not).

A new state category is created for each device, with each device's input sorted into that category
(though some actions in Touch Portal haven't implemented state sorting yet).

Note that _States_ are only created once an input has been detected. When a new device starts reporting, no states are created until
that device generates some report. So, from the previous example if the first button on a joystick is pressed, only 1 state is created,
for that particular button, even if the joystick actually has more than one button.  Pressing a second button on the controller, or moving
an axis, etc, will add a new state for that button/axis/etc.

The value of each state depends on what type of device and input is being used.

* **Buttons** and **keys** only have 2 possible values, `1` when pressed and `0` when released.
* Controller **axis** values are in the range of `0.0` through `1.0` (middle position is `0.5`).
* Controller **hats** / directional pads usually report the value as an integer for each of 8 directions and `0` when centered.
  This may vary by controller -- the plugin always sends whatever the device reported.
* **Movement** and **scrolling** states will have two numeric values separated by a comma (`,`), representing the X and Y coordinates of the
  movement/scroll action. The Touch Portal "split string and get Nth value" can be useful for breaking these up, or other string manipulation
  actions.

Also check the **Events** below for an alternative way to detect device inputs. They provide a lot more data about each input,
in a structured format.

## Events

All events use the Touch Portal v4.3+ feature of having "local states" available within the event handlers,
meaning when one of these events is used in a button or global event setup.
These can be used like other Touch Portal values in various other actions and will show up in the "local" variable types list when used inside an event handler.

All the available local states for each event are described below.

### Plugin Events

#### `Plugin State Changed`
When the plugin runnings state changes between Stopped, Starting, and Started.

Event Local States:

| Name | Type | Description |
| ---- | ---- | ----------- |
| New State | string | One of: `Stopped`, `Starting`, or `Started` |
||||

#### `Current Page Changed`
Sent when the current page on a Touch Portal client device changes.<br>
This is just a convenience event (which should be built into TP, really). It can be used for detecting when a page has been loaded (eg. to refresh data, start reporting, etc).

| State Name | Type | Description |
| ---------- | ---- | ----------- |
| New Page Name | string | Name of the page that was loaded, including the full folder path, if any. |
| Previous Page Name | string | Name of the page that was un-loaded, including any folder path. |
| Device Name | string | Name of TP device on which the page is loaded. |
| Device ID | string | ID of TP device on which the page is loaded. |
||||

### Device Events

#### Common local state data sent with _all_ device events:

| State Name | Type | Description |
| ---------- | ---- | ----------- |
| Device Name | string | Name of device sending the event. |
| Device Type | string | The device type name ("Joystick", "Gamepad", "Keyboard", etc) |
| Device Type ID | integer | The device type numeric ID (TODO: add mapping list) |
||||

### Device Status Events

#### `Device Status Event`
This event is triggered when a supported input device's status changes. Status changes include:
* `Found` - A new device was discovered on the system.<br/>
  This can heppen when the plugin first starts (when it scans for supported devices), or once running and a new device is attached to the system.
  Also sent when manually re-scanning devices using _Plugin Control Actions -> Rescan System Devices_ action.
* `Removed` - When a device that was previously found is removed from the system.<br/>
  This can happen when detaching a plug-n-play device from the system or initiating a manual re-scan and a device is no longer found.
* `Started` - Input reporting has started for a device.
* `Stopped` - Input reporting for a device stopped.

Local state data:

| State Name | Type | Description |
| ---------- | ---- | ----------- |
| Device Status | string | New status of the device; One of: "Found", "Removed", "Started", or "Stopped". |


### Device Input Events
Each event type has additional local state data relevant to the event type.

#### `Button Event`
Sent when a button on a device is pressed or released. Button events can be generated by any device with buttons (but not keyboards),
which includes game controllers as well as pointing devices.

| State Name | Type | Description |
| ---------- | ---- | ----------- |
| Button Index | integer | Button number being pressed/released. Indexes start at 1. |
| Button State | boolean | Button pressed state, `1` for pressed and `0` for released (not pressed). |
| X Position   | decimal | Horizontal position of pointing device on which the button was activated (eg. a mouse or pen), if any. |
| Y Position   | decimal | Vertical position of pointing device. The X and Y coordinates are always zero for controllers. |
||||

#### `Axis Event`
Sent when a controller-type device axis value changes (joystick, gamepad, etc).

| State Name | Type | Description |
| ---------- | ---- | ----------- |
| Axis Index | integer | Number of the axis being moved. Indexes start at 1. |
| Axis Value | decimal | Value of the axis in the range of `0.0` through `1.0` (middle position is `0.5`). |
||||

#### `Hat Event`
Sent when a controller-type device POV hat value changes (joystick, gamepad, etc).

| State Name | Type | Description |
| ---------- | ---- | ----------- |
| Hat Index | integer | Number of the hat being moved. Indexes start at 1. |
| Hat Value | integer | Reported value of the hat control.<br> Most controllers report this as integer values for each of 8 directions and `0` when centered. |
||||

#### `Scroll Event`
Sent when relative scrolling input is detected, for example a mouse wheel or a trackball-like control. Scroll events always
have a relative amount of motion in X and/or Y coordinates, meaning how much they have been moved in each direction since the
last time the input was reported. They also sometimes have absolute position coordinates indicating where on the screen the scroll
occurred, for example the mouse position at the time the wheel was moved.

| State Name | Type | Description |
| ---------- | ---- | ----------- |
| Control Index | integer | Index of physical scrolling control (wheel/ball) on the device. Indexes start at 1. Mice usually only have one. |
| Relative X Movement | decimal | Horizontal movement since last reported scroll event. For mice wheels this is generally `-1` or `1` indicating one step to left or right, respectively. |
| Relative Y Movement | decimal | Vertical movement since last reported scroll event. For mice wheels this is generally `-1` or `1` indicating one step up (away from user) or down, respectively. |
| X Position   | decimal | Horizontal position of pointing device on which the scrolling control was used (eg. mouse pointer), if any. |
| Y Position   | decimal | Vertical position of pointing device on which the scrolling control was used (eg. mouse pointer), if any. |
||||

#### `Motion Event`
These inputs are typically generated by a pointing device, such as a mouse, touchpad, etc. They always have X and Y values of the reported position, usually
in "desktop" coordinates (which could be negative).
They will usually also have relative X and Y values, meaning how much they have been moved in each direction since the
last time the input was reported.

To determine status of which button(s) are pressed during the motion, refer to the corresponding button States for the same device.
For example, a mouse will have a "Button 1" State which will be either `1` or `0` (for pressed or not), so checking the value of that State inside
the motion Event handler will tell you if the button is down or not.<br/>
If keyboard reporting is also enabled, then the same technique can be used to determine keyboard modifiers.

| State Name | Type | Description |
| ---------- | ---- | ----------- |
| Control Index | integer | Index of movement sensor on device. Indexes start at 1. Most devices would only have one. |
| X Position   | decimal | Last reported horizontal position of movement. Typically in desktop coordinates, which may be negative for multi-display setups. |
| Y Position   | decimal | Last reported vertical position of movement. Typically in desktop coordinates, which may be negative for multi-display setups. |
| Relative X Movement | decimal | Horizontal movement since last reported move event. Positive value indicate movement to the right, negative to the left. |
| Relative Y Movement | decimal | Vertical movement since last reported move event. Positive value indicate downward movement, negative is upward. |
||||

#### `Key Event`
These are keyboard events, generated for each press and release of a key. Each key has several attributes attached to it, since parsing
key strokes isn't always trivial.

The most basic value is the "Key Code" which uniquely and consistently identifies each physical key on most keyboards
(regardless of layout or language). The codes follow a convention used by various standards and software, such as the HID and USB keyboard specifications.<br/>
Here's one reference to what key each code represents (they call them "scancode" here but never mind that): https://wiki.libsdl.org/SDL3/SDL_Scancode

Each key will also have a "name," which is what would typically be shown on the key caps of a US English QWERTY keyboard. These are essentially text
equivalents of the Key Codes, portable across layouts and languages.

Neither the Key Code nor the Key Name distinguish between "modified" keys, meaning the same code & name are reported for the <kbd>1</kbd> key, for example,
even if <kbd>SHIFT</kbd> is also pressed -- which would actually produce an exclamation mark on US keyboards. Same for the number pad -- those keys are always reported
as "Keypad 1" or "Kaypad 6", for example, regardless of the current state of the <kbd>NUM LOCK</kbd> key.
Likewise, the letter keys, <kbd>A</kbd> for example, are always named in the upper case, regardless of if <kbd>SHIFT</kbd> or <kbd>CAPS LOCK</kbd> are also in effect.

Each key event will also have a "text" value, which is what the key would actually produce when typed. This (should) take into account any
pressed/toggled modifier keys, as well as the actual keyboard layout and language. So an un-shifted <kbd>A</kbd> would be lower case `a`, and a shifted <kbd>1</kbd>
would be `!` (to use previous examples). Or on my "English - International" layout, <kbd>AltGr + 1</kbd> produces `¡`.<br>
Note that some keys don't produce any text at all (cursor keys for example), and some produce "control characters" like backspace and carriage return.

Each key event has a `1` or `0` value for the `down` state, indicating if the key is being pressed or released, respectively.
There is also a `repeating` flag to indicate if the button down event is a result of the keyboard auto-repeat function (vs. the initial press).

Lastly there is the "Native Key Code" value which is going to be platform-specific.<br/>
On Windows these are the well-established "Virtual-Key", or "VK", codes listed here: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

The general modifiers currently in effect can be retrieved from the corresponding "Keyboard - Modifier [key]" and "Keyboard - Toggle [key]" States created by the
plugin for the keyboard device when it first starts reporting. They cover <kbd>SHIFT</kbd>, <kbd>CTRL</kbd>, <kbd>ALT/OPT</kbd>, and <kbd>WIN/CMD/META</kbd> modifiers
and <kbd>CAPS LOCK</kbd>, <kbd>NUM LOCK</kbd>, and <kbd>SCROLL LOCK</kbd> toggle keys.
These states have values of `1` or `0` to indicate if the modifier is enabled (held down or toggled on) or not.
These States do _not_ distinguish between left and right modifier keys.

The plugin will also create States for the individual left and right modifier keys (eg. "Left Shift" and "Right Shift") as it does for any other key press/release.
These States can be used to determine which specific modifiers are being pressed at the moment (if their value is `1`), or any other key for that matter.
This allows very specific keyboard shortcut-like functionality.

Here's the full list of local values available in the event handler:

| State Name | Type | Description |
| ---------- | ---- | ----------- |
| Key Code   | integer | Unique code for the physical key being pressed, regardless of layout, language, or modifiers. |
| Key Name   | string | Unique name for the physical key being pressed, regardless of layout, language, or modifiers. |
| Key Text   | string | Text that would be produced by the key, taking into account layout, language, and modifiers. May be empty or a control character. |
| Is Down    | boolean | Value will be `1` if the key is being pressed (or held down) and `0` when it is being released. |
| Is Repeating | boolean | Value will be `1` if the key event is generated by an auto-repeating key, and `0` on initial press (and on release). |
| Native Key Code | integer | Platform-specific key code for the key. This will vary by operating system (and possibly subsystem being used, eg. on Linux). |
||||


## Report Filter Format

The device event reporting filter (for _Set Device Report Filter_ action) has a special format that must be followed:
* Each control type on a device is represented by a letter (these also correspond to the generated device events, as described previously):
  * `a` - Axis (joystick/controller)
  * `b` - Button (on any kind of device with buttons)
  * `h` - Hat, or "direction pad" (joystick/controller)
  * `k` - Keyboard Key (uses Key Code as index, see **Key Event** for details)
  * `m` - Movement (mouse/pointer)
  * `s` - Scroll (mouse wheel/trackball)
* The letter may be followed by a number or a numeric range. This represents the index of the control as reported by the given device,
for example the button or axis number or key code. Index values always start at `1` (a zero index is invalid).<br>
Valid syntax options (where `#` represents a numeric integer value):
  * `#` - A single index, for example `b16` for button 16 on a controller, `a4` for the fourth axis, etc.
  * `#-#` - A numeric range, inclusive of both values. For example: `b1-16` includes buttons 1,2,3,...,16, or `a2-3` would specify axes 2 and 3.
  * `*` (or nothing) - Specifies a wildcard, meaning all controls of that type. For example `b*` would mean all buttons, and is equivalent to simply `b`.
* If the _first_ character is `!` (exclamation mark) then the filter will _exclude_ whatever control(s) it specifies.
Otherwise it will include _only_ reports for the specified controls, but not others of the same type. For example:
  * `!b1-6` - Means do _not_ send reports for the first 6 buttons, but do send button events for all other buttons.
  * `b1-6` - Means _do_ send reports for the first 6 buttons, but not any others.
* Multiple filters are specified by separating them with a comma (`,`) or semicolon (`;`) (spaces are allowed also). Examples:
  * `b1-8, a1, h1` - Means send reports only for buttons 1 through 8, the first axis, and the first hat of a joystick/gamepad.
  * `b16-32, !a, !h` - Only send reports for buttons 16 through 32, ignoring all axis and hat events.
* The filter order can matter, for example to exclude controls which were included within a broader range:
  * `b1-32, !b7-16` - Send reports for buttons 1 through 32, except for buttons 7 through 16. If the order of these filters were switched, then all 32 buttons
  would be reported, since the inclusive filter overrides the exclusive one.
* By default each type of a control (event) on a device is reported, _unless_ that control type has any _inclusive_ filter applied
(and the control index doesn't match the filter), or is removed by an explicit _exclusive_ filter.<br>
For example, for a joystick with 8 axes, 32 buttons and 4 hats:
  * `a1-4, !b32` - Would report on axes 1 through 4 (but not 5-8), all buttons except 32, and all 4 hats (because they're not filtered at all).
  * `!a2-6, b1-8, !h` - Would report on axes 1, 7, and 8, only the first 8 buttons, and no hats.
