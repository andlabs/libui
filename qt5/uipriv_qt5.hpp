
#ifndef __LIBUI_UIPRIV_QT5_HPP__
#define __LIBUI_UIPRIV_QT5_HPP__

#include "../ui.h"
#include "../ui_qt5.h"
#include "../common/uipriv.h"

extern void initAlloc(void);
extern void uninitAlloc(void);

// text
class QString;
char *uiQt5StrdupQString(const QString &string);

// control
class QObject;
struct uiQt5Control : public uiControl
{
	QObject *qobject;
	enum {
		DefaultFlags=0,
		DeleteControlOnQObjectFree=0x1,
		SuppressValidatationNag=0x2
	};
	uint32_t flags;
};
extern uiQt5Control *uiValidateQt5Control(uiControl *control);

// control to qobject
template <typename T=QObject>
T *uiValidateAndCastObjTo(uiControl *control)
{
	if (auto qt5Control = uiValidateQt5Control(control)) {
		return dynamic_cast<T*>(qt5Control->qobject);
	}
	return nullptr;
}
struct uiWindow : public uiQt5Control {};

// qobject to control
extern uiQt5Control *uiFindQt5ControlForQObject(const QObject *qobject);

// alloc control
extern uiQt5Control *uiAllocQt5Control(uint32_t typesig, const char *typenamestr, QObject *qobject, uint32_t flags = uiQt5Control::DefaultFlags);
#define uiAllocQt5ControlType(type, widget, flags) static_cast<type*>(uiAllocQt5Control(type ## Signature, #type, widget, flags))

// menu
class QMenuBar;
extern QMenuBar *uiQt5FindMainMenuBar();

#endif // __LIBUI_UIPRIV_QT5_HPP__
