// 24 november 2015
#include <vcclr.h>
#define EXPORT __declspec(dllexport)
#include "wpf.h"

#using <System.dll>
#using <WindowsBase.dll>
#using <PresentationCore.dll>
#using <PresentationFramework.dll>
using namespace System;
using namespace System::ComponentModel;
using namespace System::Windows;

ref class windowEvents {
public:
	wpfWindow *w;
	// TODO the using namespace above doesn't work?
	void closingEvent(Object ^sender, CancelEventArgs ^e);
};

struct wpfWindow {
	gcroot<Window ^> window;
	gcroot<windowEvents ^> events;
	void (*onClosing)(wpfWindow *, void *);
	void *onClosingData;
};

void windowEvents::closingEvent(Object ^sender, CancelEventArgs ^e)
{
	(*(this->w->onClosing))(this->w, this->w->onClosingData);
}

wpfWindow *wpfNewWindow(const char *title, int width, int height)
{
	wpfWindow *w;

	w = new wpfWindow;

	w->window = gcnew Window();
	w->window->Title = gcnew String(title);
	w->window->Width = width;
	w->window->Height = height;

	w->events = gcnew windowEvents();
	w->events->w = w;
	w->window->Closing += gcnew CancelEventHandler(w->events, &windowEvents::closingEvent);

	w->window->Show();

	return w;
}

void wpfWindowOnClosing(wpfWindow *w, void (*f)(wpfWindow *w, void *data), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

static gcroot<Application ^> app;

// wpfInit() is in sta.c; see that or details.
extern "C" void initWPF(void)
{
	app = gcnew Application();
}

void wpfRun(void)
{
	app->Run();
}

void wpfQuit(void)
{
	app->Shutdown();
}
