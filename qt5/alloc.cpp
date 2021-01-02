
#include "uipriv_qt5.hpp"

#include <QHash>
#include <QThread>
#include <QDebug>

#include "stdlib.h"

static QHash<void*,const char *> allocations;

#ifdef QT_DEBUG
// not going to put a lock on this, just simple sanity check since the rest of the code doesn't do much about thread safety
static QThread *uiThread = nullptr;
#endif

void initAlloc(void)
{
#ifdef QT_DEBUG
	uiThread = QThread::currentThread();
#endif
}

static void checkThread(const char *func)
{
#ifdef QT_DEBUG
	if (QThread::currentThread() != uiThread) {
		qWarning("Invoking %s on different thread from the one used for initialization, corruption will occur!", func);
	}
#endif
}

void uninitAlloc(void)
{
	checkThread(QT_MESSAGELOG_FUNC);
	if (!allocations.isEmpty()) {
		qWarning("Some data was leaked; either you left a uiControl lying around or there's a bug in libui itself. Leaked data:");
		for (auto it = allocations.constBegin(), e = allocations.constEnd(); it != e; ++it) {
			auto widget = uiValidateAndCastObjTo<QObject>((uiControl*)it.key());
			qWarning() << it.key() << ':' << it.value() << ':' << widget;
		}
	}
}

void *uiAlloc(size_t size, const char *type)
{
	checkThread(QT_MESSAGELOG_FUNC);
	auto p = malloc(size);
	allocations[p] = type;
	return p;
}

void *uiRealloc(void *p, size_t new_size, const char *type)
{
	checkThread(QT_MESSAGELOG_FUNC);

	if (!allocations.remove(p)) {
		qFatal("Reallocing of invalid/non-existent allocation: %p type: %s", p, type);
		return nullptr;
	}

	p = realloc(p, new_size);
	allocations[p] = type;
	return p;
}

void uiFree(void *p)
{
	checkThread(QT_MESSAGELOG_FUNC);
	if (!allocations.remove(p)) {
		qWarning("Ignoring freeing of invalid/non-existent allocation: %p", p);
		return;
	}

	free(p);
}
