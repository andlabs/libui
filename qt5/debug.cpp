
#include "uipriv_qt5.hpp"

#include <QDebug>

void realbug(const char *file, const char *line, const char *func, const char *prefix, const char *format, va_list ap)
{
	qFatal("[libui] %s:%s:%s() %s%s", file, line, func, prefix, qPrintable(QString::vasprintf(format,ap)));
}
