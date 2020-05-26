// 28 may 2019
#import "test_darwin.h"
#import "thread.h"

// TODO rename to FunctionsFailBeforeInit?
#define allcallsCase(f, ...) \
	TestNoInit(CallBeforeInitIsProgrammerError_ ## f) \
	{ \
		void *ctx; \
		ctx = beginCheckProgrammerError("attempt to call " #f "() before uiInit()"); \
		f(__VA_ARGS__); \
		endCheckProgrammerError(ctx); \
	}
#include "allcalls_darwin.h"
#undef allcallsCase

// TODO rename to FunctionsFailOnWrongThread?
#define allcallsCase(f, ...) \
	static void threadTest ## f(void *data) \
	{ \
		f(__VA_ARGS__); \
	} \
	Test(CallOnWrongThreadIsProgrammerError_ ## f) \
	{ \
		threadThread *thread; \
		threadSysError err; \
		void *ctx; \
		ctx = beginCheckProgrammerError("attempt to call " #f "() on a thread other than the GUI thread"); \
		err = threadNewThread(threadTest ## f, NULL, &thread); \
		if (err != 0) \
			TestFatalf("error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err)); \
		err = threadThreadWaitAndFree(thread); \
		if (err != 0) \
			TestFatalf("error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err)); \
		endCheckProgrammerError(ctx); \
	}
#include "allcalls_darwin.h"
#undef allcallsCase
