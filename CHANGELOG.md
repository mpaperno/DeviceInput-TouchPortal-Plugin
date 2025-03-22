# Change Log
**Device Input Plugin for Touch Portal**: changes by version number and release date.

---
## 1.0.0-beta3 (1.0.0.2) - 22-Mar-2025
* Fixed reporting of keyboard media key events.
* Key text value should now properly reflect the active keyboard layout (language) of current top application window (or system-wide layout if no window is available).
  Fixes having to restart the plugin for layout changes to take effect.

---
## 1.0.0-beta2 (1.0.0.1) - 21-Mar-2025
* Added ability to select device(s) using free-form name or type matching in _Device Control Actions_ and _Set Device Report Filter_ actions.
* Added "Wheel" type for "First" and "Default" device selection/assignments.
* Controller button State names are now padded with zeros out to 3 digits (eg."Button 001") to provide better sorting in TP State lists.

---
## 1.0.0-beta1 (1.0.0.0) - 16-Mar-2025
- Initial release for Windows.
