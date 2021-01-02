
#include "uipriv_qt5.hpp"

#include <QMainWindow>

#include <QCloseEvent>
#include <QMenuBar>
#include <QLayout>

#include <functional>

class WindowWidget : public QMainWindow
{
public:
	std::function<void (QCloseEvent *event)> onClosing;
	void closeEvent(QCloseEvent *event)
	{
		if (onClosing) {
			onClosing(event);
		}
	}
};

char *uiWindowTitle(uiWindow *w)
{
	if (auto window = uiValidateAndCastObjTo<WindowWidget>(w)) {
		return uiQt5StrdupQString(window->windowTitle());
	}
	return nullptr;
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	if (auto window = uiValidateAndCastObjTo<WindowWidget>(w)) {
		window->setWindowTitle(QString::fromUtf8(title));
	}
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	if (auto window = uiValidateAndCastObjTo<WindowWidget>(w)) {

		if (f) {
			window->onClosing = [f,w,data](QCloseEvent *event){
				if (f(w,data)) {
					// eat the event and destroy the control
					// normally such behavior would be achived with
					// setAttribute(Qt::WA_DeleteOnClose, true)
					// but we need to behave consistently
					event->accept();
					w->Destroy(w);
				}
			};
		} else {
			// clear callback
			window->onClosing = nullptr;
		}
	}
}

void uiWindowSetChild(uiWindow *w, uiControl *child)
{
	if (auto window = uiValidateAndCastObjTo<WindowWidget>(w)) {
		auto obj = uiValidateAndCastObjTo<QObject>(child);
		if (window->centralWidget()) {
			window->centralWidget()->deleteLater();
		}
		if (auto layout = qobject_cast<QLayout*>(obj)) {
			auto widget = new QWidget;
			widget->setLayout(layout);
			window->setCentralWidget(widget);
		} else if (auto widget = qobject_cast<QWidget*>(obj)) {
			window->setCentralWidget(widget);
		} else {
			qWarning("object is neither layout nor widget");
		}
	}
}

int uiWindowMargined(uiWindow *w)
{
	qWarning("TODO %p", (void*)w);
	return 0;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	qWarning("TODO %p, %d", (void*)w, margined);
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	auto window = new WindowWidget;

	window->setWindowTitle(title);
	window->resize(width,height);

	if (hasMenubar) {
		window->setMenuBar(uiQt5FindMainMenuBar());
	}

	return uiAllocQt5ControlType(uiWindow, window, uiQt5Control::DeleteControlOnQObjectFree);
}
