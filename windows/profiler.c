// 10 may 2015
#include "uipriv_windows.h"

static FILE *fprof = NULL;

static DWORD WINAPI profilerThread(LPVOID th)
{
	HANDLE thread = (HANDLE) th;
	LARGE_INTEGER counter;
	CONTEXT ctxt;

	// TODO check for errors
	if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) == 0)
		complain("error setting thread priority in profilerThread()");
	for (;;) {
		if (SuspendThread(thread) == (DWORD) (-1))
			complain("error suspending thread in profilerThread()");
		QueryPerformanceCounter(&counter);
		ZeroMemory(&ctxt, sizeof (CONTEXT));
		ctxt.ContextFlags = CONTEXT_CONTROL;
		if (GetThreadContext(thread, &ctxt) == 0)
			complain("error getting thread context in profilerThread()");
		fprintf(fprof, "%I64X %I64d\n",
			(DWORD64) (ctxt.Eip),
			counter.QuadPart);
		fflush(fprof);
		if (ResumeThread(thread) == (DWORD) (-1))
			complain("error resuming thread in profilerThread()");
		Sleep(100);
	}
	return 0;
}

void initprofiler(HANDLE thread)
{
	fprof = fopen("profiler.out", "w");
	if (fprof == NULL) {
		fprintf(stderr, "error opening profiler output file\n");
		abort();
	}
	if (CreateThread(NULL, 0, profilerThread, thread, 0, NULL) == NULL)
		complain("error creating profiler thread");
}
