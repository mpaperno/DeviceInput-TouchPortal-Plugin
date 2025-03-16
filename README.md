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
# Documentation & Examples

Please check the Wiki!
https://github.com/mpaperno/DeviceInput-TouchPortal-Plugin/wiki


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
