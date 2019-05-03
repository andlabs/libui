// 23 april 2019
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define WINVER			0x0600
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include "timer.h"

static HRESULT lastErrorCodeToHRESULT(DWORD lastError)
{
	if (lastError == 0)
		return E_FAIL;
	return HRESULT_FROM_WIN32(lastError);
}

static HRESULT lastErrorToHRESULT(void)
{
	return lastErrorCodeToHRESULT(GetLastError());
}

static HRESULT WINAPI hrWaitForMultipleObjectsEx(DWORD n, const HANDLE *objects, BOOL waitAll, DWORD timeout, BOOL alertable, DWORD *result)
{
	SetLastError(0);
	*result = WaitForMultipleObjectsEx(n, objects, waitAll, timeout, alertable);
	if (*result == WAIT_FAILED)
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrSuspendThread(HANDLE thread)
{
	DWORD ret;

	SetLastError(0);
	ret = SuspendThread(thread);
	if (ret == (DWORD) (-1))
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrGetThreadContext(HANDLE thread, LPCONTEXT ctx)
{
	BOOL ret;

	SetLastError(0);
	ret = GetThreadContext(thread, ctx);
	if (ret == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrSetThreadContext(HANDLE thread, CONST CONTEXT *ctx)
{
	BOOL ret;

	SetLastError(0);
	ret = SetThreadContext(thread, ctx);
	if (ret == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrPostThreadMessageW(DWORD threadID, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL ret;

	SetLastError(0);
	ret = PostThreadMessageW(threadID, uMsg, wParam, lParam);
	if (ret == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrResumeThread(HANDLE thread)
{
	DWORD ret;

	SetLastError(0);
	ret = ResumeThread(thread);
	if (ret == (DWORD) (-1))
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrDuplicateHandle(HANDLE sourceProcess, HANDLE sourceHandle, HANDLE targetProcess, LPHANDLE targetHandle, DWORD access, BOOL inherit, DWORD options)
{
	BOOL ret;

	SetLastError(0);
	ret = DuplicateHandle(sourceProcess, sourceHandle,
		targetProcess, targetHandle,
		access, inherit, options);
	if (ret == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrCreateWaitableTimerW(LPSECURITY_ATTRIBUTES attributes, BOOL manualReset, LPCWSTR name, HANDLE *handle)
{
	SetLastError(0);
	*handle = CreateWaitableTimerW(attributes, manualReset, name);
	if (*handle == NULL)
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrCreateEventW(LPSECURITY_ATTRIBUTES attributes, BOOL manualReset, BOOL initialState, LPCWSTR name, HANDLE *handle)
{
	SetLastError(0);
	*handle = CreateEventW(attributes, manualReset, initialState, name);
	if (*handle == NULL)
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrSetWaitableTimer(HANDLE timer, const LARGE_INTEGER *duration, LONG period, PTIMERAPCROUTINE completionRoutine, LPVOID completionData, BOOL resume)
{
	BOOL ret;

	SetLastError(0);
	ret = SetWaitableTimer(timer, duration, period, completionRoutine, completionData, resume);
	if (ret == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT __cdecl hr_beginthreadex(void *security, unsigned stackSize, unsigned (__stdcall *threadProc)(void *arg), void *threadProcArg, unsigned flags, unsigned *thirdArg, uintptr_t *handle)
{
	DWORD lastError;

	// _doserrno is the equivalent of GetLastError(), or at least that's how _beginthreadex() uses it.
	_doserrno = 0;
	*handle = _beginthreadex(security, stackSize, threadProc, threadProcArg, flags, thirdArg);
	if (*handle == 0) {
		lastError = (DWORD) _doserrno;
		return lastErrorCodeToHRESULT(lastError);
	}
	return S_OK;
}

static HRESULT WINAPI hrSetEvent(HANDLE event)
{
	BOOL ret;

	SetLastError(0);
	ret = SetEvent(event);
	if (ret == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

static HRESULT WINAPI hrWaitForSingleObject(HANDLE handle, DWORD timeout)
{
	DWORD ret;

	SetLastError(0);
	ret = WaitForSingleObject(handle, timeout);
	if (ret == WAIT_FAILED)
		return lastErrorToHRESULT();
	return S_OK;
}

timerTime timerMonotonicNow(void)
{
	LARGE_INTEGER qpc;

	QueryPerformanceCounter(&qpc);
	return qpc.QuadPart;
}

timerDuration timerTimeSub(timerTime end, timerTime start)
{
	LARGE_INTEGER qpf;
	timerDuration qpnsQuot, qpnsRem;
	timerTime c;
	timerDuration ret;

	QueryPerformanceFrequency(&qpf);
	qpnsQuot = timerSecond / qpf.QuadPart;
	qpnsRem = timerSecond % qpf.QuadPart;
	c = end - start;

	ret = ((timerDuration) c) * qpnsQuot;
	ret += (c * qpnsRem) / qpf.QuadPart;
	return ret;
}

// note: the idea for the SetThreadContext() nuttery is from https://www.codeproject.com/Articles/71529/Exception-Injection-Throwing-an-Exception-in-Other

#if defined(_AMD64_)
#define timerprivReentrant 1
#define timerprivCall
#elif defined(_ARM_)
// TODO figure out how to set arguments properly to avoid this
#define timerprivReentrant 0
#define timerprivCall
#elif defined(_ARM64_)
// TODO figure out how to set arguments properly to avoid this
#define timerprivReentrant 0
#define timerprivCall
#elif defined(_X86_)
#define timerprivReentrant 1
#define timerprivCall __fastcall
#elif defined(_IA64_)
// TODO figure out how to set arguments properly to avoid this
#define timerprivReentrant 0
#define timerprivCall
#else
#error unknown CPU architecture; cannot create CONTEXT objects for CPU-specific Windows test code
#endif

struct timeoutParams {
	jmp_buf retpos;
	HANDLE timer;
	HANDLE finished;
	HANDLE targetThread;
	DWORD targetThreadID;
	HRESULT hr;
};

#if timerprivReentrant

static void timerprivCall onTimeout(struct timeoutParams *p)
{
	longjmp(p->retpos, 1);
	// TODO flag this code should not be reached
}

static HRESULT setupNonReentrance(struct timeoutParams *p)
{
	return S_OK;
}

static void teardownNonReentrance(void)
{
	// do nothing
}

#else

static DWORD timeoutParamsSlot;
static HRESULT timeoutParamsHRESULT = S_OK;
static INIT_ONCE timeoutParamsOnce = INIT_ONCE_STATIC_INIT;

static void timerprivCall onTimeout(void)
{
	struct timeoutParams *p;

	p = (struct timeoutParams *) TlsGetValue(timeoutParamsSlot);
	longjmp(p->retpos, 1);
	// TODO flag this code should not be reached
}

static BOOL CALLBACK timeoutParamsSlotInit(PINIT_ONCE once, PVOID param, PVOID *ctx)
{
	SetLastError(0);
	timeoutParamsSlot = TlsAlloc();
	if (timeoutParamsSlot == TLS_OUT_OF_INDEXES)
		timeoutParamsHRESULT = lastErrorToHRESULT();
	return TRUE;
}

static HRESULT setupNonReentrance(struct timeoutParams *p)
{
	SetLastError(0);
	if (InitOnceExecuteOnce(&timeoutParamsOnce, timeoutParamsSlotInit, NULL, NULL) == 0)
		return lastErrorToHRESULT();
	if (timeoutParamsHRESULT != S_OK)
		return timeoutParamsHRESULT;
	if (TlsGetValue(timeoutParamsSlot) != NULL)
		return UI_E_ILLEGAL_REENTRANCY;
	SetLastError(0);
	if (TlsSetValue(timeoutParamsSlot, p) == 0)
		return lastErrorToHRESULT();
	return S_OK;
}

static void teardownNonReentrance(void)
{
	TlsSetValue(timeoutParamsSlot, NULL);
}

#endif

static void redirectToOnTimeout(CONTEXT *ctx, struct timeoutParams *p)
{
#if defined(_AMD64_)
	ctx->Rip = (DWORD64) onTimeout;
	ctx->Rcx = (DWORD64) p;
#elif defined(_ARM_)
	ctx->Pc = (DWORD) onTimeout;
#elif defined(_ARM64_)
	ctx->Pc = (DWORD64) onTimeout;
#elif defined(_X86_)
	ctx->Eip = (DWORD) onTimeout;
	ctx->Ecx = (DWORD) p;
#elif defined(_IA64_)
	// TODO verify that this is correct
	ctx->StIIP = (ULONGLONG) onTimeout;
#endif
}

static void criticalCallFailed(const char *func, HRESULT hr)
{
	fprintf(stderr, "*** internal error in timerRunWithTimeout(): %s failed: 0x%08X\n", func, hr);
	abort();
}

static unsigned __stdcall timerThreadProc(void *data)
{
	struct timeoutParams *p = (struct timeoutParams *) data;
	HANDLE objects[2];
	CONTEXT ctx;
	DWORD which;
	HRESULT hr;

	objects[0] = p->timer;
	objects[1] = p->finished;
	p->hr = hrWaitForMultipleObjectsEx(2, objects,
		FALSE, INFINITE, FALSE, &which);
	if (p->hr != S_OK)
		// act as if we timed out; the other thread will see the error
		which = WAIT_OBJECT_0;
	if (which == WAIT_OBJECT_0 + 1)
		// we succeeded; do nothing
		return 0;

	// we timed out (or there was an error); signal it
	hr = hrSuspendThread(p->targetThread);
	if (hr != S_OK)
		criticalCallFailed("SuspendThread()", hr);
	ZeroMemory(&ctx, sizeof (CONTEXT));
	ctx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
	hr = hrGetThreadContext(p->targetThread, &ctx);
	if (hr != S_OK)
		criticalCallFailed("GetThreadContext()", hr);
	redirectToOnTimeout(&ctx, p);
	hr = hrSetThreadContext(p->targetThread, &ctx);
	if (hr != S_OK)
		criticalCallFailed("SetThreadContext()", hr);
	// and force the thread to return from GetMessage(), if we are indeed in that
	// and yes, this is the way to do it (https://devblogs.microsoft.com/oldnewthing/?p=16553, https://devblogs.microsoft.com/oldnewthing/20050405-46/?p=35973, https://devblogs.microsoft.com/oldnewthing/20080528-00/?p=22163)
	hr = hrPostThreadMessageW(p->targetThreadID, WM_NULL, 0, 0);
	if (hr != S_OK)
		criticalCallFailed("PostThreadMessageW()", hr);
	hr = hrResumeThread(p->targetThread);
	if (hr != S_OK)
		criticalCallFailed("ResumeThread()", hr);
	return 0;
}

timerSysError timerRunWithTimeout(timerDuration d, void (*f)(void *data), void *data, int *timedOut)
{
	volatile struct timeoutParams p;
	MSG msg;
	volatile uintptr_t timerThreadValue = 0;
	volatile HANDLE timerThread = NULL;
	LARGE_INTEGER duration;
	HRESULT hr;

	*timedOut = 0;
	ZeroMemory(&p, sizeof (struct timeoutParams));

	hr = setupNonReentrance(&p);
	if (hr != S_OK)
		goto out;

	// to ensure that the PostThreadMessage() above will not fail because the thread doesn't have a message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	hr = hrDuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &(p.targetThread),
		0, FALSE, DUPLICATE_SAME_ACCESS);
	if (hr != S_OK) {
		p.targetThread = NULL;
		goto out;
	}
	p.targetThreadID = GetCurrentThreadId();

	hr = hrCreateWaitableTimerW(NULL, TRUE, NULL, &(p.timer));
	if (hr != S_OK) {
		p.timer = NULL;
		goto out;
	}

	hr = hrCreateEventW(NULL, TRUE, FALSE, NULL, &(p.finished));
	if (hr != S_OK) {
		p.finished = NULL;
		goto out;
	}

	if (setjmp(p.retpos) == 0) {
		hr = hr_beginthreadex(NULL, 0, timerThreadProc, p, 0, NULL, &timerThreadValue);
		if (hr != S_OK) {
			timerThread = NULL;
			goto out;
		}
		timerThread = (HANDLE) timerThreadValue;

		duration.QuadPart = d / 100;
		duration.QuadPart = -duration.QuadPart;
		hr = hrSetWaitableTimer(p.timer, &duration, 0, NULL, NULL, FALSE);
		if (hr != S_OK)
			goto out;

		(*f)(t, data);
		*timedOut = 0;
	} else if (p.hr != S_OK) {
		hr = p.hr;
		goto out;
	} else
		*timedOut = 1;
	hr = S_OK;

out:
	if (timerThread != NULL) {
		// if either of these two fail, we cannot continue because the timer thread might interrupt us later, screwing everything up
		hr = hrSetEvent(p.finished);
		if (hr != S_OK)
			criticalCallFailed("SetEvent()", hr);
		hr = hrWaitForSingleObject(timerThread, INFINITE);
		if (hr != S_OK)
			criticalCallFailed("WaitForSingleObject()", hr);
		CloseHandle(timerThread);
	}
	if (p.finished != NULL)
		CloseHandle(p.finished);
	if (p.timer != NULL)
		CloseHandle(p.timer);
	if (p.targetThread != NULL)
		CloseHandle(p.targetThread);
	teardownNonReentrance();
	return (timerSysError) hr;
}

timerSysError timerSleep(timerDuration d)
{
	HANDLE timer;
	LARGE_INTEGER duration;
	HRESULT hr;

	duration.QuadPart = d / 100;
	duration.QuadPart = -duration.QuadPart;
	hr = hrCreateWaitableTimerW(NULL, TRUE, NULL, &timer);
	if (hr != S_OK)
		return (timerSysError) hr;
	hr = hrSetWaitableTimer(timer, &duration, 0, NULL, NULL, FALSE);
	if (hr != S_OK) {
		CloseHandle(timer);
		return (timerSysError) hr;
	}
	hr = hrWaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
	return (timerSysError) hr;
}
