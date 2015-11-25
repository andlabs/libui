// 25 november 2015
#include "uipriv_wpf.hpp"

static gcroot<Application ^> *app;

void initWPF(void)
{
	app = new gcroot<Application ^>();
	*app = gcnew Application();
}

void uninitWPF(void)
{
	delete app;
}

void uiMain(void)
{
	(*app)->Run();
}

void uiQuit(void)
{
	(*app)->Shutdown();
}
