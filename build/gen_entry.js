#!/usr/bin/env node

// Generates entry.tp JSON file.
// usage: node gen_entry.js [-v <plugin.version.numbers>] [-o <output/path or - for stdout>] [-d]
// A version number is required; By default it is read from the version.json file if it is present in this folder (generated by CMake from version.json.in).
// -d (dev/debug mode) will exclude the plugin_start commands in the TP file, for running the binary separately.
// Default output path depends on -d flag; if present assumes dist/Debug, otherwise dist/Release.

const fs = require('fs');
const path = require('path');
// import { fs } from 'fs';
// import { path } from 'path';

const buildInfo = fs.existsSync("./version.json") ? JSON.parse(fs.readFileSync("./version.json", "utf8")) : {};

// Defaults
var VERSION = buildInfo.VERSION_STR;
var VERSION_NUM = parseInt(buildInfo.VERSION_NUM, 16);
var OUTPUT_PATH = "";
var DEV_MODE = false;
var PLUGIN_ID = buildInfo.PLUGIN_ID;
var STATE_NAME_PREFIX = buildInfo.STATE_NAME_PREFIX;

// Const
const SYSTEM_NAME = buildInfo.SYSTEM_NAME;
const SHORT_NAME = buildInfo.SHORT_NAME;
// const FULL_NAME = buildInfo.FULL_NAME;
const HOMEPAGE_URL = buildInfo.HOMEPAGE_URL;
const DESCRIPTION = buildInfo.DESCRIPTION;

// Handle CLI arguments
for (let i=2; i < process.argv.length; ++i) {
  const arg = process.argv[i];
  if      (arg === "-v") VERSION = process.argv[++i];
  else if (arg === "-o") OUTPUT_PATH = process.argv[++i];
  else if (arg === "-d") DEV_MODE = true;
}

// Validate the version
if (!VERSION) {
  console.error("No plugin version number, cannot continue :( \n Use -v <version.number> argument.");
  process.exit(1);
}

// Create integer version number from dotted notation in form of ((MAJ << 24) | (MIN << 16) | (PATCH << 8) | BUILD)
// Each version part is limited to the range of 0-99.
var iVersion = VERSION_NUM || 0;
if (!iVersion) {
  for (const part of VERSION.split('-', 1)[0].split('.', 4))
    iVersion = iVersion << 8 | (parseInt(part) & 0xFF);
}

// Set default output path if not specified, based on debug/release type.
if (!OUTPUT_PATH)
  OUTPUT_PATH = path.join(__dirname, "..", "dist", (DEV_MODE ? "Debug" : buildInfo.PLATFORM_OS));

// More constants, because DRY

const ICON_PATH = '%TP_PLUGIN_FOLDER%' + SYSTEM_NAME + '/tp_icon.png';
const DEVICE_EVENT_STATE_ID = "InputDevice";  // used as ID prefix for device events "local states"
// List of device types for which "First" and "Default" choices are presented to the user for device selection
const DEFAULT_AND_FIRST_DEV_TYPES = [ "Controller", "Gamepad", "Joystick", "Throttle", "Wheel" ];


// --------------------------------------
// Define the base entry.tp object here

const entry_base =
{
  api: 10,
  version: parseInt(iVersion.toString(16)),
  name: SHORT_NAME,
  id: PLUGIN_ID,
  plugin_start_cmd: DEV_MODE ? undefined : 'sh %TP_PLUGIN_FOLDER%' + SYSTEM_NAME + '/start.sh',
  plugin_start_cmd_windows: DEV_MODE ? undefined : '"%TP_PLUGIN_FOLDER%' + SYSTEM_NAME + '/bin/' + SYSTEM_NAME + '"',
  configuration: {
    colorLight: "#473866",
    colorDark: "#14101D",
    parentCategory: "input"
  },
  settingsDescription: DESCRIPTION +
    " For more details please visit the plugin's home page at: " + HOMEPAGE_URL + "\n" +
    "Plugin Version: " + VERSION,
  settings: [
    {
      name: "Send Device Reports as States",
      type: "switch",
      default: "on",
      readOnly: false,
			tooltip: {
				body: "When enabled, each device event creates & updates a Touch Portal plugin State, with a value for the active control (axis, button, key, etc)." +
          "For greater efficiency, these states can be disabled, for example if only using the (more flexible) Events system to handle input device updates."
			},
    },
    {
      name: "Send Device Reports as Events",
      type: "switch",
      default: "on",
      readOnly: false,
			tooltip: {
				body: "When enabled, each device event sends a corresponding Touch Portal plugin Event, with local state values relevant to the event type." +
          "For greater efficiency, these events can be disabled, for example if only using the plugin States system to handle input device updates."
			},
    },
  ],
  categories: [
    {
      id: PLUGIN_ID + ".cat.actions",
      name: SHORT_NAME,
      imagepath: ICON_PATH,
      states: [],
      actions: [],
      connectors: [],
      events: []
  },
  {
      id: PLUGIN_ID + ".cat.plugin",
      name: "Plugin Status",
      imagepath: ICON_PATH,
      states: [],
      actions: [],
      connectors: [],
      events: []
    },
    {
      id: PLUGIN_ID + ".cat.devices",
      name: "Device Information",
      imagepath: ICON_PATH,
      states: [],
      actions: [],
      connectors: [],
      events: []
    },
    {
      id: PLUGIN_ID + ".cat.assigned",
      name: "Device Assignments",
      imagepath: ICON_PATH,
      states: [],
      actions: [],
      connectors: [],
      events: []
    },
  ]
};


// Default category to place actions and connectors into.
const category = entry_base.categories[0];


// Some space characters useful for forcing specific spacing in action formats/descriptions.
const SP = " ";   // non-breaking narrow space U+202F (TP ignores "no-break space" U+00AD)
const EN = " ";  // en quad space U+2000  (.5em wide)
const EM = " "; // em quad space U+2001  (1em wide)

// --------------------------------------
// Helper functions

// Replaces {N} placeholders with N value from args array/tuple.
String.prototype.format = function (args) {
  if (!Array.isArray(args))
    args = new Array(args);
  return this.replace(/{([0-9]+)}/g, function (match, index) {
    return typeof args[index] == 'undefined' ? match : args[index];
  });
};

// Functions for adding actions/connectors.

function addState(id, desc, def = "", choices = null, cat = 2)
{
  const state = {
    id: PLUGIN_ID + ".state." + id,
    type: "text",
    desc : desc,
    default: def,
  };

  if (choices) {
    state.valueChoices = choices;
    state.type = "choice";
  }
  // push to 2nd category
  entry_base.categories[cat].states.push(state);
}

function addEvent(id, name, format, stateId = "", states = null, choices = null, cat = 0)
{
  const event = {
    id: PLUGIN_ID + ".event." + id,
    name: name,
    format: STATE_NAME_PREFIX + ": " + format,
    type: "communicate",
    valueType: "text",
    valueStateId: !stateId ? "" : PLUGIN_ID + ".state." + stateId,
  };

  if (choices) {
    event.valueChoices = choices;
    event.valueType = "choice";
  }
  if (states)
    event.localstates = states

  entry_base.categories[cat].events.push(event);
}

function addAction(id, name, descript, format, data, hold = false) {
  const action = {
    id: PLUGIN_ID + '.act.' + id,
    prefix: STATE_NAME_PREFIX,
    name: name,
    type: "communicate",
    tryInline: true,
    description: STATE_NAME_PREFIX + ": " + descript,
    format: String(format).format(data?.map(d => `{$${d.id}$}`)),
    hasHoldFunctionality: hold,
    data: data ? data.map(a => ({...a})) : []
  }
  addVersionData(id, action.data);
  category.actions.push(action);
}

function makeActionLinesObj(format, hold = false) {
  const key = hold ? "onhold" : "action";
  return {
    [key]: [
      {
        language: "default",
        data: format.map(l => ({ lineFormat: l })),
        // suggestions: { lineIndentation: 20, }
      },
    ]
  };
}

function addActionWithLines(id, name, descript, format = [], data = null, hold = false)
{
  const dataMapArry = data?.map(d => `{$${d.id}$}`) ?? [];
  const lines = format.map(f => String(f).format(dataMapArry));
  lines.unshift(STATE_NAME_PREFIX + ": " + descript);
  const linesObj = makeActionLinesObj(lines);
  if (hold)
    Object.assign(linesObj, makeActionLinesObj(lines, true));
  const action = {
    id: PLUGIN_ID + '.act.' + id,
    name: name,
    type: "communicate",
    lines: linesObj,
    data: data ? data.map(a => ({...a})) : []
  }
  addVersionData(id, action.data);
  category.actions.push(action);
}

// note that 'description' field is not actually used by TP as of v3.1
function addConnector(id, name, descript, format, data) {
  const action = {
    id: PLUGIN_ID + '.conn.' + id,
    name: name,
    description: descript,
    format: String(format).format(data.map(d => `{$${d.id}$}`)),
    data: data ? data.map(a => ({...a})) : []
  }
  addVersionData(id, action.data);
  category.connectors.push(action);
}

function addVersionData(id, data) {
  data.push(makeActionData(id + ".version", "number", "v", iVersion));
}

// Functions which create action/connector data members.

function makeActionData(id, type, label = "", deflt = "") {
  return {
    id: PLUGIN_ID + '.act.' + id,
    type: type,
    label:  label,
    default: deflt
  };
}

function makeTextData(id, label, dflt = "") {
  return makeActionData(id, "text", label, dflt + '');
}

function makeFileData(id, label, dflt = "") {
  return makeActionData(id, "file", label, dflt + '');
}

function makeChoiceData(id, label, choices, dflt) {
  const d = makeActionData(id, "choice", label, typeof dflt === "undefined" ? choices[0] : dflt);
  d.valueChoices = choices;
  return d;
}

function makeNumericData(id, label, dflt, min, max, allowDec = true) {
  const d = makeActionData(id, "number", label, dflt);
  d.allowDecimals = allowDec;
  d.minValue = min;
  d.maxValue = max;
  return d;
}

// Functions which create event state members.

function formatDeviceEventStateId(id, state) {
  return DEVICE_EVENT_STATE_ID + "." + (id ? id + "." : "") + state;
}

function makeDeviceEventBaseData(id /* , evType = false */)
{
  const data = [
    {
      id: formatDeviceEventStateId(id, "device.name"),
      name: "Device Name"
    },
    {
      id: formatDeviceEventStateId(id, "device.type"),
      name: "Device Type"
    },
    {
      id: formatDeviceEventStateId(id, "device.typeId"),
      name: "Device Type ID"
    }
  ];
  // if (evType) {
  //   data.push({
  //     id: formatDeviceEventStateId(id, "Event"),
  //     name: "Event Type"
  //   });
  // }
  return data;
}

// --------------------------------------
// Create plugin states & events

function createPluginStates()
{
  addState("pluginState", "Plugin running state (Stopped/Starting/Started)", "Unknown", ["Stopped", "Starting", "Started", "Unknown"], 1);
  addState("currentPage", "Name of Page currently active on TP device", "", null, 1);
}

function createPluginEvents()
{
  let id = "pluginState";
  let evStates = [
    {
      id: SYSTEM_NAME + ".RunState",
      name: "New State"
    }
  ];
  addEvent(id, "Plugin State Changed", "When the plugin runnings state changes to $val", "pluginState", evStates, ["Stopped", "Starting", "Started"]);

  id = "pageChange";
  evStates = [
    {
      id: SYSTEM_NAME + ".PageChange.PageName",
      name: "New Page Name"
    },
    {
      id: SYSTEM_NAME + ".PageChange.PreviousPage",
      name: "Previous Page Name"
    },
    {
      id: SYSTEM_NAME + ".PageChange.DeviceName",
      name: "Device Name"
    },
    {
      id: SYSTEM_NAME + ".PageChange.DeviceId",
      name: "Device ID"
    }
  ];
  addEvent(id, "Current Page Changed", "When the current page on a Touch Portal device changes", "", evStates);
}

// --------------------------------------
// Create device states & events

function addFirstDeviceState(name) {
  addState("assigned.first." + name.toLowerCase(), "First " + name + " - Name of first discovered " + name + " type device", "", null, 3);
}
function addDefaultDeviceState(name) {
  addState("assigned.default." + name.toLowerCase(), "Default " + name + " - Name of configured default " + name + " type device", "", null, 3);
}

function createDeviceStates()
{
  addState("deviceList", "List of detected input device names and types");
  addState("reportingDeviceList", "List of devices with active reports");
  addState("lastAddedDevice", "Name and type of most recently found device");
  addState("lastRemovedDevice", "Name and type of most recently removed device");
  addState("deviceReportStarted", "Name and type of most recent device for which reporting started");
  addState("deviceReportStopped", "Name and type of most recent device for which reporting stopped");
  addState("deviceStatusChange", "Status of a device changed (triggers \"Any Device Status Changed\" event)");

  for (type of DEFAULT_AND_FIRST_DEV_TYPES) {
    addFirstDeviceState(type);
    addDefaultDeviceState(type);
  }
}

function createDeviceEvents()
{
  let id = ""; // "DeviceEvent";
  let states = makeDeviceEventBaseData(id);
  states.push({ id: formatDeviceEventStateId(id, "device.status"), name: "Device Status (Found/Removed/Started/Stopped)" });
  addEvent("deviceEvent",  "Device Status Event",  "When device status (Found/Removed/Started/Stopped) changes", null, states);

  // addEvent("deviceAdded",   "Input Device Added",      "When a new input device is detected", "lastAddedDevice", makeDeviceEventBaseData("Added"));
  // addEvent("deviceRemoved", "Input Device Removed",    "When an input device is removed", "lastRemovedDevice", makeDeviceEventBaseData("Removed"));
  // addEvent("reportStarted", "Input Reporting Started", "When device input reporting starts", "deviceReportStarted", makeDeviceEventBaseData("Started"));
  // addEvent("reportStopped", "Input Reporting Stopped", "When device input reporting stops", "deviceReportStopped", makeDeviceEventBaseData("Stopped"));

  createControllerDeviceEvents();
  createDeviceKeyEvent();
  createDeviceScrollEvent();
  createDeviceMotionEvent();

  addEvent("deviceStatusChange",  "Any Device's Status Changed",  "When any Input Device's status changes to: $val", "deviceStatusChange", null, ["Found","Removed","Started","Stopped"]);
}

function createControllerDeviceEvents()
{
  let id = "AxisEvent";
  let states = makeDeviceEventBaseData(id);
  states.push({ id: formatDeviceEventStateId(id, "index"), name: "Axis Index" });
  states.push({ id: formatDeviceEventStateId(id, "value"), name: "Axis Value" });
  addEvent("deviceAxis", "Device Axis Event", "When device axis value changes", "", states);

  id = "ButtonEvent";
  states = makeDeviceEventBaseData(id);
  states.push({ id: formatDeviceEventStateId(id, "index"), name: "Button Index" });
  states.push({ id: formatDeviceEventStateId(id, "state"), name: "Button State" });
  states.push({ id: formatDeviceEventStateId(id, "x"), name: "X Position" });
  states.push({ id: formatDeviceEventStateId(id, "y"), name: "Y Position" });
  addEvent("deviceButton", "Device Button Event", "When device button state changes", "", states);

  id = "HatEvent";
  states = makeDeviceEventBaseData(id);
  states.push({ id: formatDeviceEventStateId(id, "index"), name: "Hat Index" });
  states.push({ id: formatDeviceEventStateId(id, "value"), name: "Hat Value" });
  addEvent("deviceHat", "Device Hat Event", "When device hat value changes", "", states);
}

function createDeviceKeyEvent()
{
  const id = "KeyEvent";
  const states = makeDeviceEventBaseData(id);
  states.push({ id: formatDeviceEventStateId(id, "key"), name: "Key Code" });
  states.push({ id: formatDeviceEventStateId(id, "name"), name: "Key Name" });
  states.push({ id: formatDeviceEventStateId(id, "text"), name: "Key Text" });
  states.push({ id: formatDeviceEventStateId(id, "down"), name: "Is Down" });
  states.push({ id: formatDeviceEventStateId(id, "repeat"), name: "Is Repeating" });
  // states.push({ id: formatDeviceEventStateId(id, "shift"), name: "Is Shift Pressed" });
  // states.push({ id: formatDeviceEventStateId(id, "ctrl"), name: "Is Control Pressed" });
  // states.push({ id: formatDeviceEventStateId(id, "alt"), name: "Is Alt Pressed" });
  // states.push({ id: formatDeviceEventStateId(id, "caplock"), name: "Is CapsLock On" });
  // states.push({ id: formatDeviceEventStateId(id, "numlock"), name: "Is NumLock On" });
  // states.push({ id: formatDeviceEventStateId(id, "mod"), name: "Modifiers (bitmask)" });
  states.push({ id: formatDeviceEventStateId(id, "nativeKey"), name: "Native Key Code" });
  // states.push({ id: formatDeviceEventStateId(id, "nativeCode"), name: "Native Scan Code" });
  addEvent("deviceKey", "Device Key Event", "When device key state changes", "", states);
}

function createDeviceScrollEvent()
{
  const id = "ScrollEvent";
  const states = makeDeviceEventBaseData(id);
  states.push({ id: formatDeviceEventStateId(id, "index"), name: "Control Index" });
  states.push({ id: formatDeviceEventStateId(id, "relX"), name: "Relative X Movement" });
  states.push({ id: formatDeviceEventStateId(id, "relY"), name: "Relative Y Movement" });
  states.push({ id: formatDeviceEventStateId(id, "x"), name: "X Position" });
  states.push({ id: formatDeviceEventStateId(id, "y"), name: "Y Position" });
  addEvent("deviceScroll", "Device Scroll Event", "When device scroll value changes", "", states);
}

function createDeviceMotionEvent()
{
  const id = "MotionEvent";
  const states = makeDeviceEventBaseData(id);
  states.push({ id: formatDeviceEventStateId(id, "index"), name: "Control Index" });
  states.push({ id: formatDeviceEventStateId(id, "x"), name: "X Position" });
  states.push({ id: formatDeviceEventStateId(id, "y"), name: "Y Position" });
  states.push({ id: formatDeviceEventStateId(id, "relX"), name: "Relative X Movement" });
  states.push({ id: formatDeviceEventStateId(id, "relY"), name: "Relative Y Movement" });
  // states.push({ id: formatDeviceEventStateId(id, "buttons"), name: "Buttons State (bitmask)" });
  addEvent("deviceMotion", "Device Motion Event", "When device position value changes", "", states);
}

// --------------------------------------
// Action creation functions

function addDeviceExpressionFields(id, formatLines, data) {
  formatLines.push("Match Expression:" + EM + "Device {2} {3} {4}");
  data.push(
    makeChoiceData(id + ".matchWhat", "Match Field", ["name", "type"]),
    makeChoiceData(id + ".matchType", "Match Type", ["equals", "contains", "matches wildcard", "matches RegEx"], "contains"),
    // makeChoiceData(id + ".matchWhat", "Match Field", ["", "n/a"]),
    // makeChoiceData(id + ".matchType", "Match Type", ["", "n/a"]),
    makeTextData(id + ".match", "Match Expression"),
  );
}

function addSystemActions()
{
  var id = "plugin";

  const actionChoices = [
    "Rescan System Devices",
    "Update All States & Events",
    "Send System Displays Report",
    // "Set Controller Reporting Rate",
  ];
  if (DEV_MODE)
    actionChoices.push("Shutdown");
  addAction(id, "Plugin Control Actions",
    "Plugin Control Actions.",
    // + "\nThe \"Set Controller Reporting Rate\" requires a value in milliseconds (default is 50). The other actions do not need a value.",
    // "{0} (Reporting Rate value: {1}ms)",
    "{0}",
    [
      makeChoiceData(id + ".action", "Action to Perform", actionChoices, "select an action..."),
      // makeTextData(id + ".value", "Action Value", ""),
    ]
  );

  id = "device";
  var format = ["{0} on {1}"];
  var data = [
    makeChoiceData(id + ".action", "Action to Perform", [
      "Start Reporting",
      "Stop Reporting",
      "Toggle Reporting",
      "Refresh Report",
      "Clear Report Filter",
    ], "select an action..."),
    makeChoiceData(id + ".device", "Device Name", [], "select a device..."),
  ]
  addDeviceExpressionFields(id, format, data);
  addActionWithLines(id, "Device Control Actions",
    "Perform an action on one or more devices.\n" +
      "A specific device which is currently connected can selected directly, or an expression may be used to select device(s) based on a name or type.",
    format, data
  );


  id = "filter";
  format = [
    "Set Filter: {0} Format reference: [!](a|b|h|k|m|s)[#|#-#] [, ...]",
    "On Device: {1}"
  ];
  data = [
    makeTextData(id + ".filter", "Filter Value", "b1-32, !b8-16, a1-4, !a3, !h"),
    makeChoiceData(id + ".device", "Device Name", [], "select a device..."),
  ]
  addDeviceExpressionFields(id, format, data);
  addActionWithLines(id, "Set Device Report Filter",
    "Set filter(s) for device reporting.\n" +
      "See online documentation for details." +
      "The default filter value example means: buttons 1-32 but not 8-16, axes 1-4 but not 3, no hats",
    format, data
  );

  id = "default";
  addAction(id, "Set a Default Device For Type",
    "Assign a specific device to be the default for a particular type. This allows using the \"Default ...\" type selections in the \"Device Control\" and \"Device Report Filter\" plugin actions.\n" +
      "To remove an assignment, choose the \"Remove Assigned Device\" option at the end of the name list, instead of a device name.",
    "Set device {0} as default for {1} type",
    [
      makeChoiceData(id + ".device", "Device Name", [], "select a device..."),
      makeChoiceData(id + ".type", "Device Type", DEFAULT_AND_FIRST_DEV_TYPES, "select a device type..."),
    ]
  );
}


// ------------------------
// Build the full entry.tp object for JSON dump

createDeviceStates();
createDeviceEvents();
createPluginStates();
createPluginEvents();
addSystemActions();


// Output

const output = JSON.stringify(entry_base, null, 4);
if (OUTPUT_PATH === '-') {
    console.log(output);
    process.exit(0);
}

const outfile = path.join(OUTPUT_PATH, "entry.tp");
fs.writeFileSync(outfile, output);
console.log("Wrote version", iVersion.toString(16), "output to file:", outfile);
if (DEV_MODE) {
    console.warn("!!!=== Generated DEV MODE entry.tp file ===!!!");
    process.exit(1);  // exit with error to prevent accidental usage with build scripts.
}
process.exit(0);
