// 12 january 2020
#include "uipriv_haiku.hpp"

uiprivApplication *uiprivApp;

#define uiprivInitReturnStatus(err, msg, status) uiprivInitReturnErrorf(err, "%s: %" uiprivStatustFmt, msg, status)

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

void uiprivSysMain(void)
{
	uiprivApp->Run();
}

void uiprivSysQuit(void)
{
	uiprivApp->Quit();
}

struct queueMainArgs {
	void (*f)(void *data);
	void *data;
};

// note: msg is owned by the BLooper base class and should not be freed by us
void uiprivApplication::MessageReceived(BMessage *msg)
{
	const void *data;
	const struct queueMainArgs *args;
	ssize_t size;
	status_t status;

	switch (msg->what) {
	case uiprivMsgQueueMain:
		status = msg->FindData("args", B_ANY_TYPE,
			&data, &size);
		if (status != B_OK) {
			uiprivInternalError("BMessage::FindData() failed in uiprivApplication::MessageReceived() for uiQueueMain(): %" uiprivStatustFmt, status);
			return;
		}
		args = (const struct queueMainArgs *) data;
		(*(args->f))(args->data);
		return;
	}
	BApplication::MessageReceived(msg);
}

void uiprivSysQueueMain(void (*f)(void *data), void *data)
{
	BMessage *msg;
	struct queueMainArgs args;
	status_t status;

	args.f = f;
	args.data = data;
	msg = new BMessage(uiprivMsgQueueMain);
	status = msg->AddData("args", B_RAW_TYPE,
		&args, sizeof (struct queueMainArgs), true, 1);
	if (status != B_OK) {
		uiprivInternalError("BMessage::AddData() failed in uiQueueMain(): %" uiprivStatustFmt, status);
		delete msg;
		return;
	}
	status = uiprivApp->PostMessage(msg);
	// msg is copied by PostMessage() so we can delete it here
	delete msg;
	if (status != B_OK)
		uiprivInternalError("BApplication::PostMessage() failed in uiQueueMain(): %" uiprivStatustFmt, status);
}

bool uiprivSysCheckThread(void)
{
	return find_thread(NULL) == mainThread;
}

void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal)
{
	fprintf(stderr, "*** %s: %s. %s\n", prefix, msg, suffix);
	debugger("TODO");
}
