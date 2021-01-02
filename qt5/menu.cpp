
#include "uipriv_qt5.hpp"

#include <QMenu>

#include <QApplication>
#include <QMenuBar>

struct uiMenu : public uiQt5Control {};

#ifndef uiMenuSignature
#define uiMenuSignature 0x4d656e75 // 'Menu'
#endif

struct uiMenuItem : public uiQt5Control {};

#ifndef uiMenuItemSignature
#define uiMenuItemSignature 0x4d654974 // 'MeIt'
#endif

void uiMenuItemEnable(uiMenuItem *item)
{
	if (auto action = uiValidateAndCastObjTo<QAction>(item)) {
		action->setEnabled(true);
	}
}

void uiMenuItemDisable(uiMenuItem *item)
{
	if (auto action = uiValidateAndCastObjTo<QAction>(item)) {
		action->setEnabled(false);
	}
}

static uiWindow *uiQt5FindWindow(QObject *qobject)
{
	if (qobject) {
		if (auto widget = qobject_cast<QWidget*>(qobject)) {
			if (auto topLevel = widget->topLevelWidget()) {
				return static_cast<uiWindow *>(uiFindQt5ControlForQObject(topLevel));
			}
		}
		return uiQt5FindWindow(qobject->parent());
	}
	return nullptr;
}

static uiWindow *uiQt5FindWindow(uiControl *leaf)
{
	if (auto qobject = uiValidateAndCastObjTo<QObject>(leaf)) {
		return uiQt5FindWindow(qobject);
	}
	return nullptr;
}

void uiMenuItemOnClicked(uiMenuItem *item, void (*f)(uiMenuItem *, uiWindow *, void *), void *data)
{
	if (auto action = uiValidateAndCastObjTo<QAction>(item)) {
		QObject::connect(action,&QAction::triggered, action, [f,item,data]{
			auto window = uiQt5FindWindow(item);
			f(item,window,data);
		}, Qt::UniqueConnection);
	}
}

int uiMenuItemChecked(uiMenuItem *item)
{
	if (auto action = uiValidateAndCastObjTo<QAction>(item)) {
		return action->isChecked();
	}
	return 0;
}

void uiMenuItemSetChecked(uiMenuItem *item, int checked)
{
	if (auto action = uiValidateAndCastObjTo<QAction>(item)) {
		action->setChecked(checked);
	}
}

uiMenuItem *uiMenuAppendItem(uiMenu *m, const char *name)
{
	if (auto menu = uiValidateAndCastObjTo<QMenu>(m)) {
		auto action = menu->addAction(name);

		action->setObjectName(name); // for debugging only

		return uiAllocQt5ControlType(uiMenuItem, action, uiQt5Control::DeleteControlOnQObjectFree);
	}
	return nullptr;
}

uiMenuItem *uiMenuAppendCheckItem(uiMenu *m, const char *name)
{
	auto menuItem = uiMenuAppendItem(m, name);
	if (auto action = uiValidateAndCastObjTo<QAction>(menuItem)) {
		action->setCheckable(true);
	}
	return menuItem;
}

uiMenuItem *uiMenuAppendQuitItem(uiMenu *m)
{
	uiMenuAppendSeparator(m);
	auto menuItem = uiMenuAppendItem(m, "&Quit");
	if (auto action = uiValidateAndCastObjTo<QAction>(menuItem)) {
		action->setShortcut(QKeySequence::Quit);
	}
	uiMenuItemOnClicked(menuItem, [](uiMenuItem *, uiWindow *, void *) { uiQuit(); }, nullptr);
	return menuItem;
}

uiMenuItem *uiMenuAppendPreferencesItem(uiMenu *m)
{
	uiMenuAppendSeparator(m);
	auto menuItem = uiMenuAppendItem(m, "&Preferences...");
	if (auto action = uiValidateAndCastObjTo<QAction>(menuItem)) {
		action->setShortcut(QKeySequence::Preferences);
	}
	return menuItem;
}

uiMenuItem *uiMenuAppendAboutItem(uiMenu *m)
{
	uiMenuAppendSeparator(m);
	return uiMenuAppendItem(m, "&About");
}

void uiMenuAppendSeparator(uiMenu *m)
{
	if (auto menu = uiValidateAndCastObjTo<QMenu>(m)) {
		menu->addSeparator();
	}
}

QMenuBar *uiQt5FindMainMenuBar()
{
	auto app = static_cast<QApplication*>(QCoreApplication::instance());
	for (auto widget : app->topLevelWidgets()) {
		// see if the widget is the menubar
		if (auto menuBar = qobject_cast<QMenuBar*>(widget)) {
			return menuBar;
		}
		// see if the widget owns  the menubar
		if (auto menuBar = widget->findChild<QMenuBar*>()) {
			return menuBar;
		}
	}
	// not found, create
	return new QMenuBar;
}

uiMenu *uiNewMenu(const char *name)
{
	auto menuBar = uiQt5FindMainMenuBar();
	auto menu = new QMenu(QString::fromUtf8(name), menuBar);
	menuBar->addMenu(menu);

	menu->setObjectName(name); // for debugging only

	return uiAllocQt5ControlType(uiMenu, menu, uiQt5Control::DeleteControlOnQObjectFree);
}
