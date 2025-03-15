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

#include <QDir>
#include <QEventLoop>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>

namespace Utils {

template <typename Func>
static inline void runOnThread(QThread *qThread, Func &&func)
{
	QTimer *t = new QTimer();
	t->moveToThread(qThread);
	t->setSingleShot(true);
	t->setInterval(0);
	QObject::connect(t, &QTimer::timeout, [=]()
	{
		func();
		t->deleteLater();
	});
	QMetaObject::invokeMethod(t, "start", Qt::QueuedConnection);
}

template <typename Func>
static inline void runOnThreadSync(QThread *qThread, Func &&func)
{
	QMutex m;
	QWaitCondition wc;
	runOnThread(qThread, [=, &wc]() {
		func();
		wc.notify_all();
	});
	m.lock();
	wc.wait(&m);
	m.unlock();
}

static inline void wait(std::chrono::nanoseconds nsec)
{
	QEventLoop loop;
	QTimer::singleShot(nsec, Qt::PreciseTimer, &loop, &QEventLoop::quit);
	loop.exec();
}

static inline void waitMs(int ms) {
	wait(std::chrono::milliseconds(ms));
}

static inline void waitUs(int ms) {
	wait(std::chrono::microseconds(ms));
}

}
