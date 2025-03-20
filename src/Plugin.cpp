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

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaObject>
#include <QThread>

#include "logging.h"
#include "Plugin.h"
// #include "DSE.h"
#include "device/events.h"
#include "DeviceManager.h"
#include "InputDevice.h"
#include "Logger.h"
#include "utils.h"
#include "version.h"
// #include "ConnectorData.h"

#define SETTINGS_GROUP_PLUGIN             "Plugin"
#define SETTINGS_GROUP_DEFAULT_DEVICES    "DefaultDevices"
#define SETTINGS_KEY_VERSION              "SettingsVersion"

using namespace Strings;
using namespace Devices;
using namespace Qt::Literals::StringLiterals;

static const QByteArray BoolStr[2] { "0"_ba, "1"_ba };

using ModKeyToStateIdMap = QHash<Devices::ModifierKey, uint8_t>;
Q_GLOBAL_STATIC_WITH_ARGS(const ModKeyToStateIdMap, ModKeyToStateId, ({
	{ ModifierKey::MKC_SHIFT, SID_KbdModShift },
	{ ModifierKey::MKC_CTRL,  SID_KbdModCtrl },
	{ ModifierKey::MKC_ALT,   SID_KbdModAlt },
	{ ModifierKey::MKC_GUI,   SID_KbdModGui },
	{ ModifierKey::MK_CAPS,   SID_KbdModCapLock },
	{ ModifierKey::MK_NUM,    SID_KbdModNumLock },
	{ ModifierKey::MK_SCROLL, SID_KbdModScrLock },
}))

// enum TimerEventType : quint8 {
// 	TE_None,
// };

// struct TimerData {
// 	TimerEventType type = TE_None;
// 	QVariant data;
// 	TimerData() {}
// 	TimerData(TimerEventType t, const QVariant &d) : type(t), data(d) {}
// };

// using TimerDatahash = QHash<int, TimerData>;
// Q_GLOBAL_STATIC(TimerDatahash, g_timersData)
// Q_GLOBAL_STATIC(QReadWriteLock, g_timersDataMutex)

struct EventFilter
{
	// QSet<int> include;
	// QSet<int> exclude;
	bool inclusive {false};
	bool wildcard  {false};
	QHash<int, bool> filters {};

	friend QDebug operator <<(QDebug dbg, const EventFilter &obj) {
		QDebugStateSaver saver(dbg);
		return dbg.nospace() << obj.inclusive << DBG_SEP << obj.wildcard << DBG_SEP << obj.filters;
	}
};

using eventFilter_t = EventFilter; // QHash<int, bool>;  // control index, include/exclude
using deviceEventFilter_t = QHash<EventType, eventFilter_t>;  // EventType to filter
using eventFilters_t = QHash<QByteArray, deviceEventFilter_t>;         // Device uid to event type filter
Q_GLOBAL_STATIC(eventFilters_t, g_deviceEventFilters)

bool g_startupComplete = false;
std::atomic_bool g_ignoreNextSettings = true;
std::atomic_bool g_shuttingDown = false;
std::atomic_uint_fast16_t g_systemDisplaysCount = 0;
// std::atomic_uint32_t g_errorCount = 0;

struct PluginSettings {
	bool sendSpecificStates {true};
	bool sendGenericStates {true};
	bool sendEvents {true};
} g_settings;


template <typename T>
static T formatFloatNumber(float num) {
	return T::number(num, 'f', QLocale::FloatingPointShortest);
}
static QByteArray formatFloatBA(float num) { return formatFloatNumber<QByteArray>(num); }
static QString formatFloatStr(float num) { return formatFloatNumber<QString>(num); }

static inline DeviceManager *DMI() { return DeviceManager::instance(); }

// -----------------------------------
// Plugin
// -----------------------------------

Plugin *Plugin::instance = nullptr;

Plugin::Plugin(const QString &tpHost, uint16_t tpPort, const QByteArray &pluginId, QObject *parent) :
  QObject(parent),
  m_pluginId(!pluginId.isEmpty() ? pluginId : QByteArrayLiteral(PLUGIN_ID)),
  m_pluginStateIdPrefix{m_pluginId + g_pathSep + "state"_ba + g_pathSep},
	m_pluginEventIdPrefix{m_pluginId + g_pathSep + "event"_ba + g_pathSep},
  m_pluginActIdPrefix{m_pluginId + g_pathSep + "act"_ba + g_pathSep},
  client(new TPClientQt(m_pluginId /*, this*/)),
	clientThread(new QThread())
{
	instance = this;
	// loadPluginSettings();

	client->setHostProperties(tpHost, tpPort);

	// Set up constant IDs of things we send to TP like states and choice list updates.
	for (int i = 0; i < SID_ENUM_MAX; ++i)
		m_stateIds[i] =  m_pluginStateIdPrefix + g_stateIdTokenStrings[i];
	for (int i = 0; i < EID_ENUM_MAX; ++i)
		m_eventIds[i] =  m_pluginEventIdPrefix + g_eventIdTokenStrings[i];

	for (int i = 0; i < CLID_ENUM_MAX; ++i)
		m_choiceListIds[i] =  m_pluginActIdPrefix + g_choiceListTokenStrings[i];

	connect(qApp, &QCoreApplication::aboutToQuit, this, &Plugin::quit);
	connect(this, &Plugin::loggerRotateLogs, Logger::instance(), &Logger::rotateLogs);

	connect(client, &TPClientQt::connected, this, &Plugin::onTpConnected);
	connect(client, &TPClientQt::disconnected, this, &Plugin::onClientDisconnect);
	connect(client, &TPClientQt::error, this, &Plugin::onClientError);
	connect(client, &TPClientQt::message, this, &Plugin::onTpMessage);

	connect(this, &Plugin::tpConnect, client, qOverload<>(&TPClientQt::connect));
	//connect(this, &Plugin::tpDisconnect, client, &TPClientQt::disconnect, Qt::DirectConnection);
	connect(this, &Plugin::tpStateUpdate, client, qOverload<const QByteArray &, const QByteArray &>(&TPClientQt::stateUpdate));
	// connect(this, &Plugin::tpStateUpdateStr, client, qOverload<const QByteArray &, QStringView>(&TPClientQt::stateUpdate));
	connect(this, &Plugin::tpStateCreate, client, qOverload<const QByteArray &, const QByteArray &, const QByteArray &, const QByteArray &, bool>(&TPClientQt::createState));
	// connect(this, &Plugin::tpStateRemove, client, qOverload<const QByteArray &>(&TPClientQt::removeState));
	connect(this, &Plugin::tpStateListUpdate, client, qOverload<const QByteArray &, const QStringList &>(&TPClientQt::stateListUpdate));
	connect(this, &Plugin::tpChoiceUpdate, client, qOverload<const QByteArray &, const QStringList &>(&TPClientQt::choiceUpdate));
	// connect(this, &Plugin::tpChoiceUpdate, client, qOverload<const QByteArray &, const QByteArrayList &>(&TPClientQt::choiceUpdate));
	// connect(this, &Plugin::tpChoiceUpdateInstance, client, qOverload<const QByteArray &, const QByteArray &, const QByteArrayList &>(&TPClientQt::choiceUpdate));
	connect(this, &Plugin::tpTriggerEvent, client, qOverload<const QByteArray &, const QJsonObject &>(&TPClientQt::triggerEvent));
	// connect(this, &Plugin::tpConnectorUpdateShort, client, qOverload<const QByteArray&, uint8_t>(&TPClientQt::connectorUpdate));
	connect(this, &Plugin::tpSettingUpdate, client, qOverload<const QByteArray&, const QByteArray &>(&TPClientQt::settingUpdate));
	// connect(this, &Plugin::tpNotification, client, qOverload<const QByteArray&, const QByteArray&, const QByteArray&, const QVariantList&>(&TPClientQt::showNotification));

	DeviceManager *dm = DeviceManager::instance();
	connect(dm, &DeviceManager::deviceConnected, this, &Plugin::onDeviceConnected /*, Qt::QueuedConnection*/);
	connect(dm, &DeviceManager::deviceRemoved, this, &Plugin::onDeviceRemoved /*, Qt::QueuedConnection*/);
	connect(dm, &DeviceManager::deviceNameChanged, this, &Plugin::onDeviceNameChanged);
	connect(dm, &DeviceManager::deviceStateChanged, this, &Plugin::onDeviceStateChanged);
	// connect(dm, &DeviceManager::deviceEvent, this, &Plugin::onDeviceEvent /*, Qt::QueuedConnection*/);
	connect(dm, &DeviceManager::deviceEventPtr, this, &Plugin::onDeviceEventPtr , Qt::QueuedConnection);
	connect(dm, &DeviceManager::displayDetected, this, &Plugin::updateDisplayInfoStates /*, Qt::QueuedConnection*/);
	connect(dm, &DeviceManager::displayRemoved, this, &Plugin::removeDisplayStates /*, Qt::QueuedConnection*/);

	if (clientThread) {
		client->moveToThread(clientThread);
		clientThread->start();
	}

	m_deviceListTmr.setSingleShot(true);
	m_deviceListTmr.setInterval(750);
	connect(&m_deviceListTmr, &QTimer::timeout, this, &Plugin::sendInstanceLists);

	Q_EMIT tpConnect();
	//QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
}

Plugin::~Plugin()
{
	if (!g_shuttingDown)
		quit();

	delete client;
	client = nullptr;
	delete clientThread;
	clientThread = nullptr;
	qCInfo(lcPlugin) << PLUGIN_SHORT_NAME " exiting.";
}

//void Plugin::start()
//{
//	Q_EMIT tpConnect();
//}

void Plugin::exit()
{
	if (!g_shuttingDown)
		qApp->quit();
}

void Plugin::quit()
{
	if (g_shuttingDown)
		return;
	g_shuttingDown = true;

	// QWriteLocker tl(g_timersDataMutex);
	// const QList<int> &timKeys = g_timersData->keys();
	// for (int timId : timKeys)
	// 	killTimer(timId);
	// tl.unlock();

	if (client && client->thread() != qApp->thread())
		Utils::runOnThreadSync(client->thread(), [this]() { client->moveToThread(qApp->thread()); });

	DeviceManager::instance()->deinit();

	if (client) {
		disconnect(client, nullptr, this, nullptr);
		disconnect(this, nullptr, client, nullptr);
		if (client->isConnected()) {
			updatePluginState(AT_Stopped, true);
			client->stateUpdate(m_stateIds[SID_DevicesList], QByteArray());
			client->disconnect();
		}
	}
	// savePluginSettings();
	if (clientThread) {
		clientThread->quit();
		clientThread->wait();
	}
}

void Plugin::init()
{
	DeviceManager::instance()->init();
	loadDefaultDevices();
	updatePluginState(AT_Started);
	g_startupComplete = true;
}

// void Plugin::savePluginSettings() const {
	// QSettings s;
	// s.beginGroup(SETTINGS_GROUP_PLUGIN);
	// s.setValue(SETTINGS_KEY_VERSION, APP_VERSION);
	// s.endGroup();
// }
// void Plugin::loadPluginSettings() {}

void Plugin::loadDefaultDevices()
{
	m_defaultDevices.clear();

	QSettings s;
	s.beginGroup(SETTINGS_GROUP_DEFAULT_DEVICES);
	const auto keys = s.childKeys();
	for (const auto &key : keys) {
		setDefaultDeviceForTypeName(key, s.value(key).toString(), true, false);
	}
	s.endGroup();
}

void Plugin::saveDefaultDevices() const
{
	QSettings s;
	s.beginGroup(SETTINGS_GROUP_DEFAULT_DEVICES);
	s.remove("");
	for (const auto &[type, name] : m_defaultDevices.asKeyValueRange())
		s.setValue(Devices::deviceTypeName(type), name);
	s.endGroup();
}

void Plugin::createStateWithDelay(const QByteArray &stateId, const QByteArray &parent, const QByteArray &name, const QByteArray &dflt, bool force, int delayMs) const
{
	Q_EMIT tpStateCreate(stateId, parent, /*STATE_NAME_PREFIX ": " +*/ name, dflt, force);
	// time for TP to process new state
	if (delayMs)
		QThread::msleep(delayMs);
	// Utils::waitMs(2);
}

void Plugin::updatePluginState(ActionTokens state, bool direct) const
{
	const QString stateStr = g_actionTokenStrings[state];
	const QJsonObject evData({
		{ PLUGIN_STR_EV_STATE_RUNSTATE, stateStr },
	});

	if (direct) {
		client->stateUpdate(m_stateIds[SID_PluginState].constData(), stateStr);
		client->triggerEvent(m_eventIds[EID_PluginStateChanged], evData);
	}
	else {
		Q_EMIT tpStateUpdate(m_stateIds[SID_PluginState], stateStr.toUtf8());
		Q_EMIT tpTriggerEvent(m_eventIds[EID_PluginStateChanged], evData);
	}
}

// Display (screens) info states

void Plugin::updateDisplayInfoStates(const DisplayInfo &si)
{
	const QByteArray indexName = QByteArray::number(si.index);
	const QByteArray name = QByteArray::fromStdString(si.name);
	QByteArray fullName = tr("Display ").toUtf8();
	if (!si.index)
		fullName += '(' + name + ')';
	else
		fullName += indexName + " ("_ba + name + ')';

	const auto updateSiField = [this, &indexName, &fullName](const QByteArray &field, const QString &fieldName, const QByteArray &value)
	{
		const QByteArray stateId = m_pluginStateIdPrefix + PLUGIN_STR_STATEID_DISPLAY PLUGIN_STR_PATH_SEP + indexName + g_pathSep + field;
		QWriteLocker lock(&m_mtxDeviceStates);
		if (m_deviceStates[DI_SYSTEM_SCREEN_UID][stateId].isNull())
			createStateWithDelay(stateId, fullName, fullName + " - "_ba + fieldName.toUtf8(), "", true);
		m_deviceStates[DI_SYSTEM_SCREEN_UID][stateId] = value;
		Q_EMIT tpStateUpdate(stateId, value);
	};

	updateSiField("name"_ba, tr("Name"),             name);
	updateSiField("x"_ba,    tr("Pos. - Left (x)"),  QByteArray::number(si.rect.left()));
	updateSiField("y"_ba,    tr("Pos. - Top  (y)"),  QByteArray::number(si.rect.top()));
	updateSiField("w"_ba,    tr("Size - Width"),     QByteArray::number(si.rect.width()));
	updateSiField("h"_ba,    tr("Size - Height"),    QByteArray::number(si.rect.height()));
	if (si.index > 0) {
		// these fields only for actual screens
		updateSiField("primary"_ba, tr("Is Primary"),     BoolStr[si.isPrimary] /*? "Yes"_ba : "No"_ba*/);
		updateSiField("scaling"_ba, tr("Scaling Factor"), formatFloatBA(si.scale));
		// updateSiField("number"_ba,  tr("Number"),         indexName);
	}

	// Update total displays count and (possibly) primary value states (create first if needed);
	if ((ushort)si.index > g_systemDisplaysCount) {
		g_systemDisplaysCount = (ushort)si.index;
		QByteArray *stateId = &m_stateIds[SID_DisplaysCount];
		QWriteLocker lock(&m_mtxDeviceStates);
		if (m_deviceStates[DI_SYSTEM_SCREEN_UID].value(*stateId).isNull())
			createStateWithDelay(*stateId, PLUGIN_STR_CAT_DEVICES_NAME, tr("Display Count").toUtf8(), BoolStr[0], true);
		m_deviceStates[DI_SYSTEM_SCREEN_UID][*stateId] = indexName;
		Q_EMIT tpStateUpdate(*stateId, indexName);

		if (si.isPrimary) {
			stateId = &m_stateIds[SID_DisplayPrimary];
			if (m_deviceStates[DI_SYSTEM_SCREEN_UID].value(*stateId).isNull())
				createStateWithDelay(*stateId, PLUGIN_STR_CAT_DEVICES_NAME, tr("Primary Display").toUtf8(), BoolStr[0], true);
			m_deviceStates[DI_SYSTEM_SCREEN_UID][*stateId] = indexName;
			Q_EMIT tpStateUpdate(*stateId, indexName);
		}
	}
}

void Plugin::removeDisplayStates(short nDisplay)
{
	const QByteArray indexName = QByteArray::number(nDisplay);

	const auto removeSiField = [this, &indexName](const QByteArray &field)
	{
		const QByteArray stateId = m_pluginStateIdPrefix + PLUGIN_STR_CAT_DEVICES_NAME PLUGIN_STR_PATH_SEP + indexName + g_pathSep + field;
		Q_EMIT tpStateRemove(stateId);
		QWriteLocker lock(&m_mtxDeviceStates);
		m_deviceStates.remove(stateId);
	};
	for (const auto &field : { "name"_ba, "x"_ba, "y"_ba, "w"_ba, "h"_ba, "primary"_ba, "scaling"_ba })
		removeSiField(field);
}

// Keyboard common modifier states setup, used when keyboard report starts
void Plugin::setupKeyboardStates(const InputDevice *dev) const
{
	static const QHash<uint8_t, QString> modifierNames {
		{ StateIdToken::SID_KbdModShift,   tr("Modifier - SHIFT") },
		{ StateIdToken::SID_KbdModCtrl,    tr("Modifier - CTRL") },
		{ StateIdToken::SID_KbdModCapLock, tr("Toggle - CAPS LOCK") },
		{ StateIdToken::SID_KbdModScrLock, tr("Toggle - SCROLL LOCK") },
#if defined(Q_OS_DARWIN)
		{ StateIdToken::SID_KbdModAlt,     tr("Modifier - OPT") },
		{ StateIdToken::SID_KbdModGui,     tr("Modifier - CMD") },
		// { StateIdToken::SID_KbdModNumLock, tr("CLEAR") },
#else
		{ StateIdToken::SID_KbdModAlt,     tr("Modifier - ALT") },
		{ StateIdToken::SID_KbdModNumLock, tr("Toggle - NUM LOCK") },
#ifdef Q_OS_WIN
		{ StateIdToken::SID_KbdModGui,     tr("Modifier - WIN") },
#else
		{ StateIdToken::SID_KbdModGui,     tr("Modifier - META") },
#endif
#endif
	};

	for (uint8_t i = StateIdToken::SID_KBDMOD_FIRST; i <= StateIdToken::SID_KBDMOD_LAST; ++i){
		// const QByteArray fullStateId = m_stateIds[i]; // + stateNameForDevice(dev) + g_pathSep + stateId;
		if (const auto &modName = modifierNames.value(i); !modName.isEmpty()) {
			const QString stateName = dev->name() + " - " + modName;
			createStateWithDelay(m_stateIds[i], dev->name().toUtf8(), (dev->name() + " - " + modName).toUtf8(), BoolStr[0], true, 0);
		}
	}
	// Utils::waitMs(1);
	QThread::msleep(1);
	// update the toggle keys
	DMI()->requestDeviceReport(dev->uid());
}

//

static QByteArray makeCleanStateId(const QString &dev) {
	static const QRegularExpression nameRx("[\\s\\W]+");
	return QString(dev).replace(nameRx, "_").toUtf8();
}

static inline QString formatDeviceAndTypeName(const InputDevice *dev)
{
	const QString typeName = Devices::deviceTypeName(dev->type());
	QString name = dev->name();
	if (name != typeName)
		name += " ("_L1 + typeName + ')';
	return name;
}

static inline QByteArray formatDeviceNamesList(const QList<InputDevice*> &devices, bool sorted = false)
{
	QStringList list;
	for (const InputDevice *dev : devices)
		list << formatDeviceAndTypeName(dev);
	if (sorted)
		list.sort();
	return list.join('\n').toUtf8();
}

static inline QByteArray formatDeviceNamesList(DeviceState state, DeviceManager::DeviceSortOrder order = DeviceManager::DiscoveryOrder) {
	return formatDeviceNamesList(DMI()->devices(state, order));
}

static inline InputDevice *findFirstDeviceOfType(DeviceTypes type) {
	return DMI()->devices(DeviceState::DS_Connected, DeviceManager::DiscoveryOrder, type, 1).value(0, nullptr);
}

void Plugin::sendInstanceLists() const
{
	static const QStringList combos {
		"--------"_L1,
		Strings::tokenToName(AT_AllReportingDevices),
		Strings::tokenToName(AT_First)   + ' ' + Devices::deviceTypeName(DeviceType::DT_Controller),
		Strings::tokenToName(AT_First)   + ' ' + Devices::deviceTypeName(DeviceType::DT_GamepadType),
		Strings::tokenToName(AT_First)   + ' ' + Devices::deviceTypeName(DeviceType::DT_JoystickType),
		Strings::tokenToName(AT_First)   + ' ' + Devices::deviceTypeName(DeviceType::DT_ThrottleType),
		Strings::tokenToName(AT_First)   + ' ' + Devices::deviceTypeName(DeviceType::DT_WheelType),
		Strings::tokenToName(AT_Default) + ' ' + Devices::deviceTypeName(DeviceType::DT_Controller),
		Strings::tokenToName(AT_Default) + ' ' + Devices::deviceTypeName(DeviceType::DT_GamepadType),
		Strings::tokenToName(AT_Default) + ' ' + Devices::deviceTypeName(DeviceType::DT_JoystickType),
		Strings::tokenToName(AT_Default) + ' ' + Devices::deviceTypeName(DeviceType::DT_ThrottleType),
		Strings::tokenToName(AT_Default) + ' ' + Devices::deviceTypeName(DeviceType::DT_WheelType),
	};

	Q_EMIT tpStateUpdate(m_stateIds[SID_DevicesList], formatDeviceNamesList(DeviceState::DS_Connected));
	// Q_EMIT tpStateUpdate(m_stateIds[SID_DevicesList], nameArry.join('\n').toUtf8());

	const QStringList controllerNames = DMI()->deviceNames(DeviceState::DS_Connected, DeviceManager::NameOrder, DeviceType::DT_Controller) << tokenToName(AT_RemoveDeviceAssignment);
	Q_EMIT tpChoiceUpdate(m_choiceListIds[CLID_DefaultDeviceDevName], controllerNames);

	const QStringList nameArry = DMI()->deviceNames(DeviceState::DS_Connected, DeviceManager::NameOrder) << combos;
	// nameArry.append(u"--------"_s);
	// nameArry.append(combos);

	Q_EMIT tpChoiceUpdate(m_choiceListIds[CLID_DeviceFilterDevName], nameArry);
	Q_EMIT tpChoiceUpdate(m_choiceListIds[CLID_DeviceCtrDevName], nameArry);

	// qCDebug(lcPlugin) << "Sent list updates" << m_stateIds[SID_DevicesList] << nameArry.join('\n');
}

void Plugin::sendDeviceInstanceUpdates(const InputDevice *dev) const
{
	if (dev->type().testFlag(DeviceType::DT_Controller)) {
		sendFirstAssignedDeviceStateUpdate(DeviceType::DT_Controller);
		if (dev->type() & DeviceType::DT_SubtypeMask)
			sendFirstAssignedDeviceStateUpdate(dev->type());
	}
}

void Plugin::sendFirstAssignedDeviceStateUpdate(Devices::DeviceTypes devType) const
{
	const QByteArray typeName = Devices::deviceTypeName(devType).toLower().toUtf8();

	const InputDevice *first = findFirstDeviceOfType(devType);
	const QByteArray stateId = m_pluginStateIdPrefix + PLUGIN_STR_STATEID_ASSIGNED PLUGIN_STR_PATH_SEP PLUGIN_STR_STATEID_FIRST PLUGIN_STR_PATH_SEP + typeName;
	Q_EMIT tpStateUpdate(stateId, !!first ? formatDeviceAndTypeName(first).toUtf8() : QByteArray());
	// qCDebug(lcPlugin) << "Sending first device type state" << stateId << "for device" << (first ? first->name() : "Device not found") << "for type" << devType;
}

void Plugin::sendDefaultAssignedDeviceStateUpdate(Devices::DeviceTypes devType) const
{
	const QByteArray typeName = Devices::deviceTypeName(devType).toLower().toUtf8();
	const QByteArray stateId = m_pluginStateIdPrefix + PLUGIN_STR_STATEID_ASSIGNED PLUGIN_STR_PATH_SEP PLUGIN_STR_STATEID_DEFAULT PLUGIN_STR_PATH_SEP + typeName;
	const QString deviceName = m_defaultDevices.value(devType);
	if (const InputDevice *dev = DMI()->deviceByName(deviceName))  // returns null if name is empty
		Q_EMIT tpStateUpdate(stateId, formatDeviceAndTypeName(dev).toUtf8());
	else
		Q_EMIT tpStateUpdate(stateId, deviceName.toUtf8());
}

void Plugin::sendFullStatusReport() const
{
	DeviceManager *dm = DeviceManager::instance();
	updatePluginState(ActionTokens::AT_Started, false);
	dm->updateDevices();
	// sendInstanceLists();
	const auto devices = dm->devices();
	for (const InputDevice *dev : devices) {
		if (dev->state() == DeviceState::DS_Reporting) {
			onDeviceReportStarted(dev);
			dm->requestDeviceReport(dev->uid());
			QThread::msleep(1);
			// Utils::waitMs(1);
		}
		else {
			onDeviceReportStopped(dev);
		}
	}
}

void Plugin::setDefaultDeviceForTypeName(const QString &typeName, const QString &deviceName, bool notify, bool save)
{
	const DeviceTypes t = Devices::deviceTypeNameToType(typeName);
	if (t == DeviceType::DT_Unknown) {
		qCWarning(lcPlugin) << "Could not resolve device type from type name" << typeName;
		return;
	}

	if (deviceName.isEmpty()) {
		m_defaultDevices.remove(t);
	}
	else {
		if (const InputDevice *dev = DMI()->deviceByName(deviceName); !!dev && !dev->type().testFlags(t)) {
			qCWarning(lcPlugin) << "The device type" << dev->type() << "doesn't match the default type it is being assigned to:" << t;
			return;
		}
		m_defaultDevices.insert(t, deviceName);
	}

	if (notify)
		sendDefaultAssignedDeviceStateUpdate(t);

	if (save)
		saveDefaultDevices();
}

// Device event handlers

static const QByteArray deviceLocalStatePrefix(const QByteArray &act, const QByteArray &state) {
	if (act.isEmpty())
		return PLUGIN_STR_EV_STATE_DEVICE PLUGIN_STR_PATH_SEP + state;
	return PLUGIN_STR_EV_STATE_DEVICE PLUGIN_STR_PATH_SEP + act + g_pathSep + state;
}

// Common local states object for all device input events
static QJsonObject deviceStatesObject(const InputDevice * const dev, const QByteArray &act)
{
	return QJsonObject({
		{ deviceLocalStatePrefix(act, "device.name"_ba),   dev->name() },
		{ deviceLocalStatePrefix(act, "device.type"_ba),   Devices::deviceTypeName(dev->type()) },
		{ deviceLocalStatePrefix(act, "device.typeId"_ba), QString::number(dev->type()) },
	});
}


void Plugin::dispatchDeviceEvent(const InputDevice *dev, ActionTokens event, StateIdToken stateId, EventIdToken /*eventId*/) const
{

	if (event == AT_Found || event == AT_Removed) {
		sendInstanceLists();
		sendDeviceInstanceUpdates(dev);
	}

	if (stateId != StateIdToken::SID_ENUM_MAX)
		Q_EMIT tpStateUpdate(m_stateIds[stateId], formatDeviceAndTypeName(dev).toUtf8());

	// triggers deviceStatusChange event
	Q_EMIT tpStateUpdate(m_stateIds[SID_DeviceStatusChange], g_actionTokenStrings[event]);

	static const QByteArray evName; // "DeviceEvent"_ba;
	QJsonObject evStates = deviceStatesObject(dev, evName);
	evStates.insert(deviceLocalStatePrefix(evName, "device.status"_ba), g_actionTokenStrings[event]);
	Q_EMIT tpTriggerEvent(m_eventIds[EID_DeviceEvent], evStates);

	// if (eventId != EventIdToken::EID_ENUM_MAX)
	// 	Q_EMIT tpTriggerEvent(m_eventIds[eventId], deviceStatesObject(dev, g_actionTokenStrings[event]));
}

void Plugin::onDeviceConnected(const QByteArray &uid) const
{
	if (InputDevice *dev = DMI()->device(uid)) {
		// sendInstanceLists();
		dispatchDeviceEvent(dev, AT_Found, SID_LastFoundDevice /*, EID_DeviceFound*/);
		return;
	}
	qCWarning(lcPlugin) << "Couldn't get device for" << uid;
}

void Plugin::onDeviceRemoved(const QByteArray &uid) const
{
	if (InputDevice *dev = DMI()->device(uid)) {
		// sendInstanceLists();
		dispatchDeviceEvent(dev, AT_Removed, SID_LastRemovedDevice /*, EID_DeviceRemoved*/);
		return;
	}
	qCWarning(lcPlugin) << "Couldn't get device for" << uid;
}

void Plugin::onDeviceReportStarted(const InputDevice *dev) const
{
	dispatchDeviceEvent(dev, ActionTokens::AT_Started, SID_DevReportStarted /*, EID_ReportStarted*/);

	if (dev->type().testFlag(DeviceType::DT_Keyboard))
		setupKeyboardStates(dev);
}

void Plugin::onDeviceReportStopped(const InputDevice *dev) const
{
	dispatchDeviceEvent(dev, ActionTokens::AT_Stopped, SID_DevReportStopped /*, EID_ReportStopped*/);
}

void Plugin::onDeviceStateChanged(const InputDevice *dev, DeviceState newState, DeviceState previousState) const
{
	if (!dev)
		return;

	if (newState == DeviceState::DS_Reporting)
		onDeviceReportStarted(dev);
	else if (previousState == DeviceState::DS_Reporting)
		onDeviceReportStopped(dev);
	else
		return;

	Q_EMIT tpStateUpdate(m_stateIds[SID_ReportingDevices], formatDeviceNamesList(DeviceState::DS_Reporting));
}

void Plugin::onDeviceNameChanged(const InputDevice *dev, const QString &/*name*/) const
{
	if (dev && dev->state() > DeviceState::DS_Seen /*&& !m_deviceListTmr.isActive()*/) {
		sendInstanceLists();
		sendDeviceInstanceUpdates(dev);
		// m_deviceListTmr.start();
	}
}

void Plugin::onDeviceEvent(const DeviceEvent &ev)
{
	if (!g_settings.sendEvents && !g_settings.sendSpecificStates /*&& !g_settings.sendGenericStates*/)
		return;

	const InputDevice *dev = ev.device ? ev.device : DMI()->device(ev.deviceUid);
	if (!dev /*|| dev->state() != DeviceState::DS_Reporting*/)
		return;

	const auto &idf = g_deviceEventFilters->find(dev->uid());
	if (idf != g_deviceEventFilters->cend()) {
		const auto &ief = idf->find(ev.type);
		if (ief != idf->cend()) {
			const EventFilter &ef = ief.value();
			if (ef.wildcard)
				return;
			const auto &cef = ef.filters.find(ev.index);
			if (cef != ef.filters.cend()) {
				if (!cef.value())
					return;
			}
			else if (ef.inclusive) {
				return;
			}
		}
	}

	QByteArray stateValue;
	// QByteArray stateId;
	// QString stateName;
	EventIdToken evId = EID_ENUM_MAX;
	QByteArray ctrlName(QByteArray::number(ev.index));
	//const auto evName = (QByteArray(QMetaEnum::fromType<Devices::EventType>().valueToKey(ev.type) + 6) + "Event"_L1).toLatin1();
	const QByteArray evName = g_deviceEventStrings[ev.type] + "Event"_ba;
	QJsonObject evStates = deviceStatesObject(dev, evName);

	switch (ev.type)
	{
		case EventType::Event_Axis: {
			const auto &aev = static_cast<const DeviceAxisEvent&>(ev);
			stateValue = formatFloatBA(aev.value);
			evId = EventIdToken::EID_DeviceAxis;
			evStates.insert(deviceLocalStatePrefix(evName, "index"_ba), ctrlName.constData());
			evStates.insert(deviceLocalStatePrefix(evName, "value"_ba), stateValue.constData());
			break;
		}
		case EventType::Event_Button: {
			const auto &aev = static_cast<const DeviceButtonEvent&>(ev);
			stateValue = BoolStr[aev.down];
			evId = EventIdToken::EID_DeviceButton;
			evStates.insert(deviceLocalStatePrefix(evName, "index"_ba), ctrlName.constData());
			evStates.insert(deviceLocalStatePrefix(evName, "state"_ba), stateValue.constData());
			evStates.insert(deviceLocalStatePrefix(evName, "x"_ba), formatFloatStr(aev.x));
			evStates.insert(deviceLocalStatePrefix(evName, "y"_ba), formatFloatStr(aev.y));
			// qCDebug(lcPlugin) << "Button Event" << aev.index << aev.down << aev.timestamp;
			break;
		}
		case EventType::Event_Hat: {
			const auto &aev = static_cast<const DeviceHatEvent&>(ev);
			stateValue = QByteArray::number(aev.value);
			evId = EventIdToken::EID_DeviceHat;
			evStates.insert(deviceLocalStatePrefix(evName, "index"_ba), ctrlName.constData());
			evStates.insert(deviceLocalStatePrefix(evName, "value"_ba), stateValue.constData());
			break;
		}
		case EventType::Event_Scroll: {
			const auto &aev = static_cast<const DeviceScrollEvent&>(ev);
			stateValue = formatFloatBA(aev.relX) + ',' + formatFloatBA(aev.relY);
			evId = EventIdToken::EID_DeviceScroll;
			evStates.insert(deviceLocalStatePrefix(evName, "index"_ba), ctrlName.constData());
			evStates.insert(deviceLocalStatePrefix(evName, "x"_ba), formatFloatStr(aev.x));
			evStates.insert(deviceLocalStatePrefix(evName, "y"_ba), formatFloatStr(aev.y));
			evStates.insert(deviceLocalStatePrefix(evName, "relX"_ba), formatFloatStr(aev.relX));
			evStates.insert(deviceLocalStatePrefix(evName, "relY"_ba), formatFloatStr(aev.relY));
			break;
		}
		case EventType::Event_Motion: {
			const auto &aev = static_cast<const DeviceMotionEvent&>(ev);
			stateValue = formatFloatBA(aev.x) + ',' + formatFloatBA(aev.y);
			evId = EventIdToken::EID_DeviceMotion;
			evStates.insert(deviceLocalStatePrefix(evName, "index"_ba), ctrlName.constData());
			evStates.insert(deviceLocalStatePrefix(evName, "x"_ba), formatFloatStr(aev.x));
			evStates.insert(deviceLocalStatePrefix(evName, "y"_ba), formatFloatStr(aev.y));
			evStates.insert(deviceLocalStatePrefix(evName, "relX"_ba), formatFloatStr(aev.relX));
			evStates.insert(deviceLocalStatePrefix(evName, "relY"_ba), formatFloatStr(aev.relY));
			// evStates.insert(deviceStatePrefix(evName, "buttons"_ba), QString::number(aev.buttons, 16).prepend("0x"));
			break;
		}
		case EventType::Event_Key: {
			const auto &aev = static_cast<const DeviceKeyEvent&>(ev);
			ctrlName = aev.name.toUtf8();
			stateValue = BoolStr[aev.down];
			evId = EventIdToken::EID_DeviceKey;
			evStates.insert(deviceLocalStatePrefix(evName, "key"_ba), QString::number(aev.key()));
			evStates.insert(deviceLocalStatePrefix(evName, "name"_ba), aev.name);
			evStates.insert(deviceLocalStatePrefix(evName, "text"_ba), aev.text);
			evStates.insert(deviceLocalStatePrefix(evName, "down"_ba), stateValue.constData());
			evStates.insert(deviceLocalStatePrefix(evName, "repeat"_ba), BoolStr[aev.repeat].constData());
			evStates.insert(deviceLocalStatePrefix(evName, "nativeKey"_ba), QString::number(aev.nativeKey));
			// evStates.insert(deviceStatePrefix(evName, "mod"_ba), QString::number(aev.modifiers, 16).prepend("0x"));
			// evStates.insert(deviceStatePrefix(evName, "nativeCode"_ba), QString::number(aev.nativeScanCode));
			// evStates.insert(deviceStatePrefix(evName, "sdlKey"_ba), QString::number(aev.sdlKey));

			if (const auto modKey = Devices::scanCodeToGeneralModifierType(aev.scancode()); modKey != ModifierKey::MK_NONE) {
				if (const uint8_t stateId = ModKeyToStateId->value(modKey))
					Q_EMIT tpStateUpdate(m_stateIds[stateId], stateValue);
			}

			break;
		}

		default:
			qCWarning(lcPlugin) << "Un-handled Device Event" << ev.timestamp << ev.type << ev.deviceType << ev.deviceUid;
			return;
	}
	// qCDebug(lcPlugin) << "Device Event" << ev.timestamp << ev.type << ev.deviceType << ev.deviceUid << dev;

	if (g_settings.sendSpecificStates /*|| g_settings.sendGenericStates*/) {
		const QByteArray stateId = QByteArray(g_deviceEventStateIds[ev.type]) + g_pathSep + ctrlName;
		const QByteArray fullStateId = m_pluginStateIdPrefix + makeCleanStateId(dev->name()) + g_pathSep + stateId;
		// const QByteArray uid(dev->uid());
		// const QString devName(dev->name());

		// QWriteLocker lock(&m_mtxDeviceStates);
		m_mtxDeviceStates.lockForRead();
		const QByteArray lastState = m_deviceStates[dev->uid()].value(stateId);
		m_mtxDeviceStates.unlock();

		// Create a new state if we didn't have a record of this one yet.
		if (lastState.isNull()) {
			// pad button names to 3 digits on controllers so states sort alphabetically
			if (ev.type == EventType::Event_Button && ev.deviceType.testFlag(DeviceType::DT_Controller)) {
				while (ctrlName.size() < 3)
					ctrlName.prepend('0');
			}
			const QByteArray stateName = (dev->name() + " - "_L1 + g_deviceEventStrings[ev.type] + ' ' + ctrlName).toUtf8();
			createStateWithDelay(fullStateId, dev->name().toUtf8(), stateName);
			// qCDebug(lcPlugin) << "Created state" << fullStateId << stateName << "for" << dev->name();
		}
		if (lastState != stateValue) {
			m_mtxDeviceStates.lockForWrite();
			m_deviceStates[dev->uid()][stateId] = stateValue;
			m_mtxDeviceStates.unlock();
			Q_EMIT tpStateUpdate(fullStateId, stateValue);
			// qCDebug(lcPlugin) << "Updated state" << fullStateId << "to" << stateValue << "evId" << m_eventIds[evId] << "for" << dev->name();
		}
	}

	if (g_settings.sendEvents && evId != EventIdToken::EID_ENUM_MAX)
		Q_EMIT tpTriggerEvent(m_eventIds[evId], evStates);
}

void Plugin::onDeviceEventPtr(const DeviceEvent *ev)
{
	if (!!ev) {
		onDeviceEvent(*ev);
		delete ev;
	}
}

// value: [!](a|b|h|k|m|s|r)[#|#-#|*] [(,|;| )...]  eg: b1-32,!b8-16, a1 a4; !h
static bool parseDeviceFilterAction(QStringView value, deviceEventFilter_t &def)
{
	static const QRegularExpression ctrlSplitRx(u"[\\s,;]+"_s);
	static const QHash<char, EventType> evMap({
		{ 'a', EventType::Event_Axis },
		{ 'b', EventType::Event_Button },
		{ 'h', EventType::Event_Hat },
		{ 'k', EventType::Event_Key },
		{ 'm', EventType::Event_Motion },
		{ 's', EventType::Event_Scroll },
		{ 'r', EventType::Event_Sensor },
	});

	if (value.isEmpty())
		return false;

	const auto controls = value.split(ctrlSplitRx, Qt::SkipEmptyParts);
	bool excl, ok;
	EventType ev;
	int rng1, rng2;
	for (auto ctrl : controls) {
		if ((excl = ctrl.at(0) == '!')) {
			ctrl.slice(1);
			if (ctrl.isEmpty())
				continue;
		}
		if ((ev = evMap.value(ctrl.first().toLatin1(), EventType::Event_Generic)) == EventType::Event_Generic)
			continue;

		EventFilter &ef = def[ev];
		if (!excl)
			ef.inclusive = true;

		ctrl = ctrl.slice(1).trimmed();
		if (ctrl.isEmpty() || ctrl.at(0) == '*') {
			ef.wildcard = true;
			qCDebug(lcPlugin) << ctrl << ef;
			continue;
		}
		const auto rangePair = ctrl.split('-');
		rng1 = rangePair.front().toInt(&ok);
		if (!ok || rng1 < 1)
			continue;
		if (rangePair.length() == 2) {
			rng2 = rangePair.at(1).toInt(&ok);
			if (!ok || rng2 < rng1)
				continue;
		}
		else {
			rng2 = rng1;
		}

		for (; rng1 <= rng2; ++rng1)
			ef.filters.insert(rng1, !excl);

		qCDebug(lcPlugin) << ctrl << ef;
	}
	return true;
}

// void Plugin::timerEvent(QTimerEvent *ev)
// {
// 	if (ev->timerId())
// 		killTimer(ev->timerId());
// 	else
// 		return;

// 	QWriteLocker tl(g_timersDataMutex);
// 	const TimerData timData = g_timersData->take(ev->timerId());
// }

void Plugin::onClientDisconnect()
{
	if (!g_shuttingDown) {
		if (!g_startupComplete)
			qCCritical(lcPlugin) << "Unable to connect to Touch Portal, shutting down now.";
		else
			qCCritical(lcPlugin) << "Unexpectedly disconnected from Touch Portal, shutting down now.";
		exit();
	}
}

void Plugin::onClientError(QAbstractSocket::SocketError /*e*/)
{
	if (g_startupComplete)
		qCCritical(lcPlugin) << "Lost connection to Touch Portal, shutting down now.";
	else
		qCCritical(lcPlugin) << "Unable to connect to Touch Portal, shutting down now.";
	exit();
}


void Plugin::onTpConnected(const TPClientQt::TPInfo &info, const QJsonObject &settings)
{
	qCInfo(lcPlugin).nospace().noquote()
		<< PLUGIN_SHORT_NAME " v" APP_VERSION_STR " Connected to Touch Portal v" << info.tpVersionString
		<< " (" << info.tpVersionCode << "; SDK v" << info.sdkVersion
		<< ") for plugin ID " << m_pluginId << " with entry.tp v" << info.pluginVersion;
	updatePluginState(AT_Starting);
	handleSettings(settings);
	init();
	// m_loadSettingsTmr.start();
}

void Plugin::onTpMessage(TPClientQt::MessageType type, const QJsonObject &msg)
{
	//qCDebug(lcPlugin) << msg;
	switch (type) {
		case TPClientQt::MessageType::action:
		case TPClientQt::MessageType::down:
		// case TPClientQt::MessageType::up:
		// case TPClientQt::MessageType::connectorChange:
			dispatchAction(type, msg);
			break;

		case TPClientQt::MessageType::broadcast: {
			if (msg.value("event"_L1).toString().compare("pageChange"_L1))
				break;

			QString pgName = msg.value("pageName"_L1).toString(); //.sliced(1).replace(".tml", ""_L1).replace('\\', '/');
			if (pgName.size() > 1)
				pgName.slice(1).replace(".tml", ""_L1).replace('\\', '/');
			if (pgName.isEmpty())
				break;
			Q_EMIT tpStateUpdate(m_stateIds[SID_TpCurrentPage], pgName.toUtf8());

			QString fromPg = msg.value("previousPageName"_L1).toString();
			if (fromPg.size() > 1)
				fromPg.slice(1).replace(".tml", ""_L1).replace('\\', '/');

			const QJsonObject evData({
				{ PLUGIN_STR_EV_STATE_PGCHANGE PLUGIN_STR_PATH_SEP "PageName",     pgName },
				{ PLUGIN_STR_EV_STATE_PGCHANGE PLUGIN_STR_PATH_SEP "PreviousPage", fromPg },
				{ PLUGIN_STR_EV_STATE_PGCHANGE PLUGIN_STR_PATH_SEP "DeviceName",   msg.value("deviceName"_L1) },
				{ PLUGIN_STR_EV_STATE_PGCHANGE PLUGIN_STR_PATH_SEP "DeviceId",     msg.value("deviceId"_L1) },
			});
			Q_EMIT tpTriggerEvent(m_eventIds[EID_TpCurrentPageChange], evData);
			break;
		}

		// case TPClientQt::MessageType::listChange: {
		// 	if (!msg.value("actionId").toString().endsWith(tokenToId(AID_DeviceControl)))
		// 		break;
		// 	if (!msg.value("listId").toString().endsWith(QLatin1String(".action")))
		// 		break;
		// 	int token = tokenFromName(msg.value("value").toString().toUtf8());
		// 	if (token != AT_Unknown) {
		// 		updateInstanceChoices(token, msg.value("instanceId").toString().toUtf8());
		// 	}
		// 	break;
		// }

		case TPClientQt::MessageType::settings:
			if (!g_ignoreNextSettings)
				handleSettings(msg);
			g_ignoreNextSettings = false;
			break;

		case TPClientQt::MessageType::closePlugin:
			qCInfo(lcPlugin) << "Got plugin close message from TP, exiting.";
			exit();
			return;

		// case TPClientQt::MessageType::notificationOptionClicked:
		// 	// passthrough to any scripts which may be listening on a callback.
		// 	Q_EMIT tpNotificationClicked(msg.value(QLatin1String("notificationId")).toString(), msg.value(QLatin1String("optionId")).toString());
		// 	break;

		default:
			break;
	}
}

void Plugin::dispatchAction(TPClientQt::MessageType type, const QJsonObject &msg)
{
	const QString actId = msg.value(type == TPClientQt::MessageType::connectorChange ? "connectorId"_L1 : "actionId"_L1).toString();
	const QVector<QStringView> actIdArry = QStringView(actId).split('.');

	if (actIdArry.length() < 7) {
		qCCritical(lcPlugin) << "Action ID is malformed for action:" << actId;
		return;
	}

	// const int handler = tokenFromName(actIdArry.at(6).toUtf8());
	// if (handler == AT_Unknown) {
	// 	qCCritical(lcPlugin) << "Unknown action handler for this plugin:" << actId;
	// 	return;
	// }

	const QByteArray action(actIdArry.at(6).toUtf8());
	const int act = tokenFromName(action);
	if (act == AT_Unknown) {
		qCCritical(lcPlugin) << "Unknown action for this plugin:" << action;
		return;
	}

	const QJsonArray data = msg.value("data"_L1).toArray();
	if (!data.size()) {
		qCCritical(lcPlugin) << "Action data missing for action:" << actId;
		return;  // we have no actions w/out data members
	}

	// const QMap<QString, QString> dataMap = TPClientQt::actionDataToMap(data, g_pathSep);
	// qint32 connVal = type == TPClientQt::MessageType::connectorChange ? msg.value("value"_L1).toInt(0) : -1;

	pluginAction(type, act, TPClientQt::actionDataToMap(data, g_pathSep), -1 /*connVal*/);

	// switch(handler) {
	// 	case AHID_Plugin:
	// 		pluginAction(type, act, dataMap, connVal);
	// 		break;

	// 	default:
	// 		return;
	// }
}

static int resolveSubAction(const QMap<QString, QString> &dataMap)
{
	int subAct = tokenFromName(dataMap.value("action").toUtf8());
	if (subAct == AT_Unknown)
		qCCritical(lcPlugin) << "Unknown Command action:" << dataMap.value("action");
	return subAct;
}

Plugin::DeviceListFromActionT Plugin::getDeviceFromActionData(const QMap<QString, QString> &dataMap)
{
	const QString name = dataMap.value("device");
	if (name.startsWith(PLUGIN_STR_MISC_ACT_DATA_PLACEHOLDER_PFX))
		return DeviceListFromActionT();

	InputDevice *dev = DMI()->deviceByName(name);
	if (!!dev)
		return DeviceListFromActionT({ dev });

	if (name == g_actionTokenStrings[AT_AllReportingDevices]) {
		const auto list = DMI()->devices(DeviceState::DS_Reporting, DeviceManager::DiscoveryOrder);
		return DeviceListFromActionT(list.cbegin(), list.cend());
	}

	const QString typeName = name.split(' ').last().trimmed();
	const DeviceTypes types = deviceTypeNameToType(typeName);
	if (types == DeviceType::DT_Unknown) {
		qCWarning(lcPlugin) << "Couldn't determine device type from type name" << typeName;
		return DeviceListFromActionT();
	}

	if (name.startsWith(Strings::tokenToName(AT_First))) {
		return DeviceListFromActionT({ findFirstDeviceOfType(types) });
	}

	if (name.startsWith(Strings::tokenToName(AT_Default))) {
		if ((dev = DMI()->deviceByName(m_defaultDevices.value(types))))
			return DeviceListFromActionT({ dev });
	}

	qCWarning(lcPlugin) << "Couldn't find any device for name or assigned type" << name;
	return DeviceListFromActionT();
}

void Plugin::pluginAction(TPClientQt::MessageType /*type*/, int act, const QMap<QString, QString> &dataMap, qint32 /*connectorValue*/)
{
	switch (act)
	{
		case AID_PluginControl: {
			const int subAct = resolveSubAction(dataMap);
			switch (subAct)
			{
				case CA_RescanDevices:
					Q_EMIT tpStateUpdate(m_stateIds[SID_DevicesList], QByteArray());
					DMI()->updateDevices();
					break;
				case CA_DisplaysReport:
					if (g_systemDisplaysCount > 0) {
						g_systemDisplaysCount = 0;
						Q_EMIT tpStateUpdate(m_stateIds[SID_DisplaysCount], BoolStr[0]);
						Q_EMIT tpStateUpdate(m_stateIds[SID_DisplayPrimary], BoolStr[0]);
					}
					DMI()->requestDeviceReport(DI_SYSTEM_SCREEN_UID);
					break;
				case CA_FullStatusUpdate:
					sendFullStatusReport();
					break;
				case CA_SetControllerRepRate: {
					const QString val = dataMap.value("value"_L1).trimmed();
					if (val.isEmpty()) {
						DMI()->resetControllerUpdateInterval();
						break;
					}
					bool ok;
					const int ms = val.toInt(&ok);
					if (ok && ms >= 5)
						DMI()->setControllerUpdateInterval(ms);
					break;
				}
				case CA_Shutdown:
					qCInfo(lcPlugin()) << "Got shutdown command, exiting.";
					exit();
					break;
				default:
					qCDebug(lcPlugin) << "No Handler defined for Plugin Control sub-action" << subAct;
					return;
			}
			break;
		}

		case AID_DeviceControl:
			if (const DeviceListFromActionT devs = getDeviceFromActionData(dataMap); !devs.isEmpty()) {
				const int subAct = resolveSubAction(dataMap);
				for (const InputDevice *dev : devs) {
					if (!dev)
						break;
					const auto devState = dev->state();
					switch (subAct)
					{
						case CA_StartReport:
							if (devState != DeviceState::DS_Reporting)
								DMI()->startDeviceReport(dev->uid());
							break;
						case CA_StopReport:
							if (devState == DeviceState::DS_Reporting)
								DMI()->stopDeviceReport(dev->uid());
							break;
						case CA_ToggleReport:
							DMI()->toggleDeviceReport(dev->uid());
							break;
						case CA_RefreshReport:
							if (devState > DeviceState::DS_Seen)
								DMI()->requestDeviceReport(dev->uid());
							break;
						case CA_ClearFilter:
							g_deviceEventFilters->remove(dev->uid());
							qCDebug(lcPlugin) << "Removed report filter for device" << dev->name();
							break;
						default:
							qCDebug(lcPlugin) << "No Handler defined for Device Control sub-action" << subAct;
							return;
					}
				}
			}
			break;

		case AID_DeviceFilter:
			if (const DeviceListFromActionT devs = getDeviceFromActionData(dataMap); !devs.isEmpty()) {
				const auto value = dataMap.value("filter"_L1).trimmed();
				deviceEventFilter_t df;
				if (!value.isEmpty() && !parseDeviceFilterAction(dataMap.value("filter"_L1), df))
					break;

				for (const InputDevice *dev : devs) {
					if (!dev)
						break;
					if (value.isEmpty()) {
						g_deviceEventFilters->remove(dev->uid());
						qCDebug(lcPlugin) << "Removed report filter for device" << dev->name();
					}
					else {
						(*g_deviceEventFilters)[dev->uid()] = df;
						qCDebug(lcPlugin) << "Added report filter" << df.values() << "for device" << dev->name();
					}
				}
			}
			break;

		case AID_DeviceDefault: {
			QString devName = dataMap.value("device");
			const QString typeName = dataMap.value("type");
			if (devName.startsWith(PLUGIN_STR_MISC_ACT_DATA_PLACEHOLDER_PFX) || typeName.startsWith(PLUGIN_STR_MISC_ACT_DATA_PLACEHOLDER_PFX))
				break;
			if (tokenFromName(devName.toUtf8()) == AT_RemoveDeviceAssignment)
				devName.clear();
			setDefaultDeviceForTypeName(typeName, devName);
		}
			break;

		default:
			qCDebug(lcPlugin) << "No Handler defined for Plugin action" << act;
			return;
	}
}

static bool stringToBool(QJsonValue val) {
	static const QRegularExpression boolRx(uR"RX((on|1|true|yes))RX"_s, QRegularExpression::DontCaptureOption);
	return val.toString().contains(boolRx);
}

void Plugin::handleSettings(const QJsonObject &settings) const
{
	// qCDebug(lcPlugin) << "Got settings object:" << settings;
	if (const QJsonValue val{settings.value(g_actionTokenStrings[ST_SendReportStates])}; !val.isUndefined())
		g_settings.sendSpecificStates = stringToBool(val);
	if (const QJsonValue val{settings.value(g_actionTokenStrings[ST_SendReportEvents])}; !val.isUndefined())
		g_settings.sendEvents = stringToBool(val);
}

#include "moc_Plugin.cpp"
