
#include "uipriv_qt5.hpp"

#include <QString>

static char *uiStrdupText(const char *s, int len)
{
	// Reminder: could use strndup, but it has poor portability
	auto copy = (char *)malloc(len+1);
	if (copy) {
		memcpy(copy, s, len);
		copy[len] = 0;
	} else {
		qCritical("Failed to allocate %d+1 bytes", len);
	}

	return copy;
}

char *uiQt5StrdupQString(const QString &string)
{
	if (string.isNull()) {
		return nullptr;
	}

	const auto utf8 = string.toUtf8();
	return uiStrdupText(utf8.constData(), utf8.length());
}

void uiFreeText(char *t)
{
	free(t);
}
