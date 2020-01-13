// 12 january 2020
#include "uipriv_haiku.hpp"

uiprivApplication *uiprivApp;

// TODO add format string warning detection to all these functions, where available
// TODO also see if we can convert this to a string, or use a known type for status_t instead of assuming it's int(32_t)
#define uiprivInitReturnStatus(err, msg, status) uiprivInitReturnErrorf(err, "%s: %d", msg, status)

static thread_id mainThread;

bool uiprivSysInit(void *options, uiInitError *err)
{
	status_t status;

	uiprivApp = new uiprivApplication("application/libui.TODO", &status);
	if (status != B_OK)
		return uiprivInitReturnStatus(err, "error creating BApplication", status);
	mainThread = find_thread(NULL);
	return true;
}

void uiMain(void)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	uiprivApp->Run();
}

// TODO if this is called beofre uiMain(), uiprivApp will be deleted; either ban doing this outright or catch this scenario
void uiQuit(void)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	uiprivApp->Quit();
}

void uiprivSysQueueMain(void (*f)(void *data), void *data)
{
//	TODO
}

bool uiprivSysCheckThread(void)
{
	return find_thread(NULL) == mainThread;
}

void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal)
{
	fprintf(stderr, "*** %s: %s. %s\n", prefix, msg, suffix);
	debugger("TODO");
	abort();		// we shouldn't reach here
}
