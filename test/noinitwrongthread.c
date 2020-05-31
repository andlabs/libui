// 28 may 2019
#include "test.h"
#include "thread.h"

#define allcallsHeader "allcalls.h"
#include "noinitwrongthreadimpl.h"

TestNoInit(CallBeforeInitIsProgrammerError_uiQueueMain)
{
	void *ctx;

	ctx = beginCheckProgrammerError("attempt to call uiQueueMain() before uiInit()");
	uiQueueMain(NULL, NULL);
	endCheckProgrammerError(ctx);
}

// no uiQueueMain() test for the wrong thread; it's supposed to be callable from any thread
