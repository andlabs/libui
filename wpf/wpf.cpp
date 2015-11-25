// 24 november 2015
#define UNICODE
#define _UNICODE
#include <windows.h>
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

void wpfInit(void)
{
	HRESULT hr;

	// see http://stackoverflow.com/questions/24348205/how-do-i-solve-this-com-issue-in-c
	// we MUST be running STA
	// .net initializes as MTA for some stupid reason
	// TODO https://msdn.microsoft.com/en-us/library/5s8ee185%28v=vs.71%29.aspx use CoInitializeEx()?
	hr = CoInitialize(NULL);
	if (hr != S_OK && hr != S_FALSE)
		DebugBreak();

	app = gcnew Application();
}

void wpfRun(void)
{
	app->Run();
	CoUninitialize();
}

void wpfQuit(void)
{
	app->Shutdown();
}
