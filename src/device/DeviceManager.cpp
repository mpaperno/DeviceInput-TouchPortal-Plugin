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

#include "DeviceManager.h"

// #include "events.h"
#include "DeviceDescriptor.h"
#include "InputDevice.h"
#include "logging.h"
#include "SDLManager.h"
#include "utils.h"

#ifdef USE_WINDOWS_HOOK
#include "WindowsDeviceManager.h"
#endif


using namespace Devices;

class DeviceManagerPublic : public DeviceManager {
	public:
		explicit DeviceManagerPublic() : DeviceManager(nullptr) {}
};

Q_GLOBAL_STATIC(DeviceManagerPublic, dmInstance)

class DeviceManagerPrivate
{
	Q_DECLARE_PUBLIC(DeviceManager)
	DeviceManagerPrivate(DeviceManager *q) : q_ptr(q)
	{
		tmrDeviceLoadDelay.setSingleShot(true);
		tmrDeviceLoadDelay.setInterval(250);
		QObject::connect(&tmrDeviceLoadDelay, &QTimer::timeout, q_ptr, [this]() { updateDeviceNames(); });
	}

	~DeviceManagerPrivate() {
		qDeleteAll(devices.values());
		delete nativeManager;
		delete sdlManager;
	}

	void initManagerIface(IApiManager *m)
	{
		q_ptr->connect(m, &IApiManager::deviceDiscovered, q_ptr, &DeviceManager::onPlatformDeviceDiscovered /*, Qt::QueuedConnection*/);
		q_ptr->connect(m, &IApiManager::deviceRemoved, q_ptr, &DeviceManager::onPlatformDeviceRemoved/*, Qt::QueuedConnection*/);
		q_ptr->connect(m, &IApiManager::deviceEvent, q_ptr, &DeviceManager::onPlatformDeviceEvent /*, Qt::QueuedConnection*/);
		q_ptr->connect(m, &IApiManager::deviceReportToggled, q_ptr, &DeviceManager::onPlatformDeviceReportToggled /*, Qt::QueuedConnection*/);
		q_ptr->connect(m, &IApiManager::displayDetected, q_ptr, &DeviceManager::displayDetected /*, Qt::QueuedConnection*/);
		q_ptr->connect(m, &IApiManager::displayRemoved, q_ptr, &DeviceManager::displayRemoved /*, Qt::QueuedConnection*/);

		m->init();
		// if (m->init())
		// 	m->scanDevices();
	}

	void deinitManagerIface(IApiManager *m)
	{
		m->deinit();
		q_ptr->disconnect(m, nullptr, q_ptr, nullptr);
		m->deleteLater();
	}

	void setDeviceNameWithInstance(InputDevice *dev) const
	{
		const auto i = dev->instance();
		const QString sfx = QString::number(i);
		if (i > 0 /*&& !dev->name().endsWith(sfx)*/)
			dev->setName(dev->descriptorName() + ' ' + sfx);
	}

	void updateDeviceNames() const
	{
		QHash<QString, InputDevice *> seenNames;
		QMultiHash<QString, InputDevice*> duped;

		for (const auto &uid : std::as_const(discoveryOrder)) {
			// for (const auto dev : d->devices.values())
			if (const auto dev = devices.value(uid); dev && dev->state() >= DeviceState::DS_Connected) {
			//for (InputDevice *dev : devices.values()) {
				// if (dev->state() < DeviceState::DS_Connected) {
				// 	// dev->resetName();
				// 	continue;
				// }
				const QString origName = dev->descriptorName();
				if (seenNames.contains(origName)) {
					auto dupes = duped.count(origName);
					if (!dupes) {
						InputDevice *firstSeen = seenNames[origName];
						duped.insert(origName, firstSeen);
						// ++dupes;
						// if (firstSeen->instance() == 0)
						firstSeen->setInstance(++dupes);
					}
					// ++dupes;
					// if (dev->instance() == 0)
					dev->setInstance(++dupes);
					duped.insert(origName, dev);
				}
				else {
					seenNames.insert(origName, dev);
				}
			}
		}

#if 0
		QHash<QString, InputDevice *> seenNames;
		QSet<InputDevice*> duped;

		for (InputDevice *dev : devices.values()) {
			// if (dev->type() & DeviceType::DT_Gamepad)
			// 	setDeviceNameWithInstance(dev);
			// else
			if (seenNames.contains(dev->name()))
				duped << dev << seenNames[dev->name()];
			else
				seenNames.insert(dev->name(), dev);
		}
#endif

		for (InputDevice *dev : std::as_const(duped)) {
			setDeviceNameWithInstance(dev);
		}
	}

	QByteArrayList discoveryOrder;  // list of device UIDs in order of discovery; devices removed from here when disconnected
	QHash<QByteArray, InputDevice *> devices;
	std::atomic_bool initComplete { false };
	std::atomic_bool globalPending { false };

	QTimer tmrDeviceLoadDelay;
	SDLManager *sdlManager = nullptr;
	IApiManager *nativeManager = nullptr;
	DeviceManager * const q_ptr;

};


DeviceManager::DeviceManager(QObject *parent) :
  QObject{parent},
  d_ptr(new DeviceManagerPrivate(this))
{
	qRegisterMetaType<DeviceDescriptor>();
	qRegisterMetaType<Devices::DeviceEvent>();
	qRegisterMetaType<Devices::DeviceEvent*>();
	qRegisterMetaType<Devices::DevicePositionEvent>();
	qRegisterMetaType<Devices::DeviceAxisEvent>();
	qRegisterMetaType<Devices::DeviceHatEvent>();
	qRegisterMetaType<Devices::DeviceKeyEvent>();
	qRegisterMetaType<Devices::DeviceButtonEvent>();
	qRegisterMetaType<Devices::DeviceMotionEvent>();
	qRegisterMetaType<Devices::DeviceScrollEvent>();
	qRegisterMetaType<Devices::DisplayInfo>();
}

DeviceManager::~DeviceManager()
{
	deinit();
	delete d_ptr;
}

DeviceManager *DeviceManager::instance() { return dmInstance; }

InputDevice *DeviceManager::device(const QByteArray &uid) const
{
	Q_DC(DeviceManager);
	return d->devices.value(uid, nullptr);
}

InputDevice *DeviceManager::deviceByName(const QString &name, Qt::MatchFlags matchFlags) const {
	return devicesByName(name, matchFlags, 1).value(0, nullptr);
}

QList<InputDevice *> DeviceManager::devicesByName(const QString &name, Qt::MatchFlags matchFlags, qsizetype maxHits) const
{
	QList<InputDevice *> list;
	if (name.isEmpty())
		return list;

	Q_DC(DeviceManager);
	if (!matchFlags.testFlag(Qt::MatchRegularExpression)) {
		const Qt::CaseSensitivity cs = (matchFlags.testFlag(Qt::MatchCaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive);

		if ((matchFlags & Qt::MatchTypeMask) == Qt::MatchExactly) {
			for (const auto dev : d->devices.values()) {
				if (!dev->name().compare(name, cs))
					list << dev;
				if (maxHits && maxHits == list.size())
					break;
			}
			return list;
		}

		// "Contains" match type
		for (const auto &uid : std::as_const(d->discoveryOrder)) {
			if (const auto dev = d->devices.value(uid); !!dev && dev->name().contains(name, cs))
				list << dev;
			if (maxHits && maxHits == list.size())
				break;
		}
		return list;
	}

	const QRegularExpression re = Utils::expressionToRegEx(name, matchFlags);
	qCDebug(lcDevices) << "Matching on name with regex" << re << "from original qry" << name;
	for (const auto &uid : std::as_const(d->discoveryOrder)) {
		if (const auto dev = d->devices.value(uid); !!dev && dev->name().contains(re))
			list << dev;
		if (maxHits && maxHits == list.size())
			break;
	}
	return list;
}

QList<InputDevice *> DeviceManager::devices(DeviceState minState, DeviceSortOrder order, DeviceTypes type, qsizetype maxHits) const
{
	Q_DC(DeviceManager);
	const bool anyType = type == DeviceType::DT_Unknown;
	if (minState <= DeviceState::DS_Seen && order == DeviceSortOrder::Unordered && anyType) {
		if (maxHits && maxHits < d->devices.size())
			return d->devices.values().mid(0, maxHits);
		return d->devices.values();
	}

	QList<InputDevice *> list;
	list.reserve(maxHits ? std::min(maxHits, d->devices.size()) : d->devices.size());

	// const auto &srcList = order == DeviceSortOrder::DiscoveryOrder ? d->discoveryOrder : d->devices.keys();
	if (order == DeviceSortOrder::DiscoveryOrder) {
		for (const auto &uid : std::as_const(d->discoveryOrder)) {
			if (const auto dev = d->devices.value(uid); !!dev && dev->state() >= minState && (anyType || dev->type().testFlags(type))) {
				list << dev;
				if (maxHits && maxHits == list.size())
					break;
			}
		}
	}
	else {
		for (const auto dev : d->devices.values()) {
			if (!!dev && dev->state() >= minState && (anyType || dev->type().testFlags(type))) {
				list << dev;
				if (maxHits && maxHits == list.size())
					break;
			}
		}
		if (order == DeviceSortOrder::NameOrder)
			std::sort(list.begin(), list.end(), [](InputDevice *d1, InputDevice *d2) { return d1->name().localeAwareCompare(d2->name()); });
	}
	return list;
}

QStringList DeviceManager::deviceNames(DeviceState minState, DeviceSortOrder order, DeviceTypes type) const
{
	Q_DC(DeviceManager);
	const bool anyType = type == DeviceType::DT_Unknown;
	QStringList list;
	list.reserve(d->devices.size());

	if (order == DeviceSortOrder::DiscoveryOrder) {
		for (const auto &uid : std::as_const(d->discoveryOrder)) {
			if (const auto dev = d->devices.value(uid); !!dev && dev->state() >= minState && (anyType || dev->type().testFlags(type)))
				list << dev->name();
		}
	}
	else {
		for (const auto dev : d->devices.values()) {
			if (!!dev && dev->state() >= minState && (anyType || dev->type().testFlags(type)))
				list << dev->name();
		}
		if (order == DeviceSortOrder::NameOrder)
			list.sort();
	}
	return list;
}

void DeviceManager::init()
{
	Q_D(DeviceManager);
	if (d->initComplete || d->globalPending)
		return;
	d->globalPending = true;

#ifdef USE_WINDOWS_HOOK
	d->nativeManager = new WindowsDeviceManager(this);
#endif
	if (d->nativeManager)
		d->initManagerIface(d->nativeManager);

	d->sdlManager = new SDLManager(this);
	d->initManagerIface(d->sdlManager);

	d->globalPending = false;
	d->initComplete = true;
	qCDebug(lcDevices) << "DeviceManager init completed";

	updateDevices();
}

void DeviceManager::deinit()
{
	Q_D(DeviceManager);
	if (!d->initComplete || d->globalPending)
		return;

	d->globalPending = true;

	if (d->sdlManager) {
		d->deinitManagerIface(d->sdlManager);
		d->sdlManager = nullptr;
	}

	if (d->nativeManager) {
		d->deinitManagerIface(d->nativeManager);
		d->nativeManager = nullptr;
	}

	d->globalPending = false;
	d->initComplete = false;
	qCDebug(lcDevices) << "DeviceManager deinit completed";
}

void DeviceManager::setControllerUpdateInterval(int ms)
{
	Q_DC(DeviceManager);
	if (d->sdlManager)
		d->sdlManager->setActiveScanInterval(ms);
}

void DeviceManager::resetControllerUpdateInterval()
{
	Q_DC(DeviceManager);
	if (d->sdlManager)
		d->sdlManager->setActiveScanInterval(d->sdlManager->defaultActiveScanInterval());
}

void DeviceManager::updateDevices()
{
	Q_D(DeviceManager);
	if (d->globalPending)
		return;
	if (!d->initComplete)
		init();

	qCDebug(lcDevices) << "Device refresh requested.";

	d->globalPending = true;

	if (d->nativeManager)
		d->nativeManager->scanDevices();
	if (d->sdlManager)
		d->sdlManager->scanDevices();

	d->globalPending = false;
}

void DeviceManager::startDeviceReport(const QByteArray &uid) const
{
	Q_DC(DeviceManager);
	if (InputDevice *dev = d->devices.value(uid); dev && dev->state() != DeviceState::DS_Reporting && dev->state() >= DeviceState::DS_Connected) {
		if (dev->api() == DeviceAPI::DA_SDL)
			d->sdlManager->connectDevice(uid);
		else if (dev->api() == DeviceAPI::DA_NATIVE && d->nativeManager)
			d->nativeManager->connectDevice(uid);
	}

}

void DeviceManager::stopDeviceReport(const QByteArray &uid) const
{
	Q_DC(DeviceManager);
	if (InputDevice *dev = d->devices.value(uid); dev && dev->state() == DeviceState::DS_Reporting) {
		if (dev->api() == DeviceAPI::DA_SDL)
			d->sdlManager->disconnectDevice(uid);
		else if (dev->api() == DeviceAPI::DA_NATIVE && d->nativeManager)
			d->nativeManager->disconnectDevice(uid);
	}
}

void DeviceManager::toggleDeviceReport(const QByteArray &uid) const
{
	Q_DC(DeviceManager);
	if (InputDevice *dev = d->devices.value(uid)) {
		if (dev->state() == DeviceState::DS_Reporting)
			stopDeviceReport(uid);
		else if (dev->state() >= DeviceState::DS_Connected)
			startDeviceReport(uid);
	}

}

void DeviceManager::requestDeviceReport(const QByteArray &uid) const
{
	Q_DC(DeviceManager);
	if (const InputDevice *dev = d->devices.value(uid); dev && dev->state() > DeviceState::DS_Seen) {
		if (dev->api() == DeviceAPI::DA_SDL && d->sdlManager)
			d->sdlManager->sendDeviceReport(uid);
		else if (dev->api() == DeviceAPI::DA_NATIVE && d->nativeManager)
			d->nativeManager->sendDeviceReport(uid);
	}
	else if (uid == DI_SYSTEM_SCREEN_UID) {
		d->sdlManager->sendDeviceReport(uid);
	}
}

void DeviceManager::onPlatformDeviceDiscovered(const DeviceDescriptor &dd)
{
	Q_D(DeviceManager);
	InputDevice *dev = d->devices.value(dd.uid);
	qCDebug(lcDevices) << "Device Discovered" << dd.uid << dev;

	if (!dev) {
		dev = new InputDevice(dd, this);
		dev->setState(DeviceState::DS_Seen);
		connect(dev, &InputDevice::nameChanged, this, &DeviceManager::onDevNameChanged);
		connect(dev, &InputDevice::stateChanged, this, &DeviceManager::onDevStateChanged);
		d->devices.insert(dd.uid, dev);

		Q_EMIT deviceDiscovered(dd.uid);
		qCDebug(lcDevices) << "Added new device" << dd;
	}

	if (dev->state() < DeviceState::DS_Connected) {
		d->discoveryOrder.append(dd.uid);
		dev->setState(DeviceState::DS_Connected);
	}

	d->tmrDeviceLoadDelay.start();
	Q_EMIT deviceConnected(dd.uid);
}

void DeviceManager::onPlatformDeviceRemoved(const QByteArray &uid)
{
	Q_D(DeviceManager);
	InputDevice *dev = d->devices.value(uid);
	qCDebug(lcDevices) << "Device Removed" << uid << dev;
	d->discoveryOrder.removeAll(uid);

	if (!dev) {
		qCWarning(lcDevices) << "Couldn't find removed device in current devices list for UID" << uid;
		return;
	}
	dev->setState(DeviceState::DS_Seen);
	Q_EMIT deviceRemoved(uid);
	dev->resetName();
	dev->setInstance(0);
}

void DeviceManager::onPlatformDeviceEvent(DeviceEvent *ev)
{
	Q_DC(DeviceManager);
	// qCDebug(lcDevices) << "Device Event" << ev->timestamp << ev->type << ev->deviceType << ev->deviceUid;
	if (InputDevice *dev = d->devices.value(ev->deviceUid) /*; dev && dev->state() == DeviceState::DS_Reporting*/) {
		ev->device = dev;
		// Q_EMIT deviceEvent(*ev);
		Q_EMIT deviceEventPtr(ev);
	}
	else {
		delete ev;
	}
}

void DeviceManager::onPlatformDeviceReportToggled(const QByteArray &uid, bool started)
{
	Q_DC(DeviceManager);
	InputDevice *dev = d->devices.value(uid);
	if (!dev)
		return;

	if (started) {
		dev->setState(DeviceState::DS_Reporting);
		Q_EMIT deviceReportStarted(uid);
	}
	else {
		dev->setState(DeviceState::DS_Connected);
		Q_EMIT deviceReportStopped(uid);
	}
}

void DeviceManager::onDevNameChanged(const QString &name)
{
	if (InputDevice *dev = qobject_cast<InputDevice*>(sender()))
		Q_EMIT deviceNameChanged(dev, name);
}

void DeviceManager::onDevStateChanged(DeviceState newState, DeviceState previousState)
{
	if (InputDevice *dev = qobject_cast<InputDevice*>(sender()))
		Q_EMIT deviceStateChanged(dev, newState, previousState);
}
