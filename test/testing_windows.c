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
#include <process.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testing.h"
#include "testingpriv.h"

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

// note: the idea for the SetThreadContext() nuttery is from https://www.codeproject.com/Articles/71529/Exception-Injection-Throwing-an-Exception-in-Other

static jmp_buf timeout_ret;

static void onTimeout(void)
{
	longjmp(timeout_ret, 1);
}

static HANDLE timeout_timer;
static HANDLE timeout_finished;
static HANDLE timeout_targetThread;
static DWORD timeout_targetThreadID;
static HRESULT timeout_hr;

static void setContextForGet(CONTEXT *ctx)
{
	ZeroMemory(ctx, sizeof (CONTEXT));
	ctx->ContextFlags = CONTEXT_CONTROL;
}

static void setContextForSet(CONTEXT *ctx)
{
#if defined(_AMD64_)
	ctx->Rip = (DWORD64) onTimeout;
#elif defined(_ARM_)
	ctx->Pc = (DWORD) onTimeout;
#elif defined(_ARM64_)
	ctx->Pc = (DWORD64) onTimeout;
#elif defined(_X86_)
	ctx->Eip = (DWORD) onTimeout;
#elif defined(_IA64_)
	// TODO verify that this is correct
	ctx->StIIP = (ULONGLONG) onTimeout;
#else
#error unknown CPU architecture; cannot create CONTEXT objects for CPU-specific Windows test code
#endif
}

static unsigned __stdcall timerThreadProc(void *data)
{
	HANDLE objects[2];
	CONTEXT ctx;
	DWORD which;
	HRESULT hr;

	objects[0] = timeout_timer;
	objects[1] = timeout_finished;
	timeout_hr = hrWaitForMultipleObjectsEx(2, objects,
		FALSE, INFINITE, FALSE, &which);
	if (timeout_hr != S_OK)
		// act as if we timed out; the other thread will see the error
		which = WAIT_OBJECT_0;
	if (which == WAIT_OBJECT_0 + 1)
		// we succeeded; do nothing
		return 0;

	// we timed out (or there was an error); signal it
	hr = hrSuspendThread(timeout_targetThread);
	if (hr != S_OK)
		testingprivInternalError("error calling SuspendThread() after timeout: 0x%08I32X", hr);
	setContextForGet(&ctx);
	hr = hrGetThreadContext(timeout_targetThread, &ctx);
	if (hr != S_OK)
		testingprivInternalError("error calling GetThreadContext() after timeout: 0x%08I32X", hr);
	setContextForSet(&ctx);
	hr = hrSetThreadContext(timeout_targetThread, &ctx);
	if (hr != S_OK)
		testingprivInternalError("error calling SetThreadContext() after timeout: 0x%08I32X", hr);
	// and force the thread to return from GetMessage(), if we are indeed in that
	// and yes, this is the way to do it (https://devblogs.microsoft.com/oldnewthing/?p=16553, https://devblogs.microsoft.com/oldnewthing/20050405-46/?p=35973, https://devblogs.microsoft.com/oldnewthing/20080528-00/?p=22163)
	hr = hrPostThreadMessageW(timeout_targetThreadID, WM_NULL, 0, 0);
	if (hr != S_OK)
		testingprivInternalError("error calling PostThreadMessage() after timeout: 0x%08I32X", hr);
	hr = hrResumeThread(timeout_targetThread);
	if (hr != S_OK)
		testingprivInternalError("error calling ResumeThread() after timeout: 0x%08I32X", hr);
	return 0;
}

void testingprivRunWithTimeout(testingT *t, const char *file, long line, int64_t timeout, void (*f)(testingT *t, void *data), void *data, const char *comment, int failNowOnError)
{
	char *timeoutstr;
	MSG msg;
	int closeTargetThread = 0;
	uintptr_t timerThread = 0;
	LARGE_INTEGER duration;
	int waitForTimerThread = 0;
	HRESULT hr;

	timeoutstr = testingNsecString(timeout);

	// to ensure that the PostThreadMessage() above will not fail because the thread doesn't have a message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	hr = hrDuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &timeout_targetThread,
		0, FALSE, DUPLICATE_SAME_ACCESS);
	if (hr != S_OK) {
		testingprivTLogfFull(t, file, line, "error getting current thread for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}
	closeTargetThread = 1;
	timeout_targetThreadID = GetCurrentThreadId();

	hr = hrCreateWaitableTimerW(NULL, TRUE, NULL, &timeout_timer);
	if (hr != S_OK) {
		testingprivTLogfFull(t, file, line, "error creating timer for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	hr = hrCreateEventW(NULL, TRUE, FALSE, NULL, &timeout_finished);
	if (hr != S_OK) {
		testingprivTLogfFull(t, file, line, "error creating finished event for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	duration.QuadPart = timeout / 100;
	duration.QuadPart = -duration.QuadPart;
	hr = hrSetWaitableTimer(timeout_timer, &duration, 0, NULL, NULL, FALSE);
	if (hr != S_OK) {
		testingprivTLogfFull(t, file, line, "error applying %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	// don't start the thread until after we call setjmp()
	hr = hr_beginthreadex(NULL, 0, timerThreadProc, NULL, CREATE_SUSPENDED, NULL, &timerThread);
	if (hr != S_OK) {
		testingprivTLogfFull(t, file, line, "error creating timer thread for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}
	waitForTimerThread = 1;

	if (setjmp(timeout_ret) == 0) {
		hr = hrResumeThread((HANDLE) timerThread);
		if (hr != S_OK) {
			testingprivTLogfFull(t, file, line, "error calling ResumeThread() to start timeout thread: 0x%08I32X", hr);
			testingTFail(t);
			waitForTimerThread = 0;
			goto out;
		}
		(*f)(t, data);
		failNowOnError = 0;		// we succeeded
	} else if (timeout_hr == S_OK) {
		testingprivTLogfFull(t, file, line, "%s timeout passed (%s)", comment, timeoutstr);
		testingTFail(t);
	} else {
		testingprivTLogfFull(t, file, line, "error running timer thread for %s timeout: 0x%08I32X", comment, timeout_hr);
		testingTFail(t);
	}

out:
	if (timerThread != 0) {
		if (waitForTimerThread) {
			hr = hrSetEvent(timeout_finished);
			if (hr != S_OK)
				testingprivInternalError("error signaling timer thread to finish for %s timeout: 0x%08I32X (this is fatal because that thread may interrupt us)", comment, hr);
			hr = hrWaitForSingleObject((HANDLE) timerThread, INFINITE);
			if (hr != S_OK)
				testingprivInternalError("error waiting for timer thread to quit for %s timeout: 0x%08I32X (this is fatal because that thread may interrupt us)", comment, hr);
		}
		CloseHandle((HANDLE) timerThread);
	}
	if (timeout_finished != NULL) {
		CloseHandle(timeout_finished);
		timeout_finished = NULL;
	}
	if (timeout_timer != NULL) {
		CloseHandle(timeout_timer);
		timeout_timer = NULL;
	}
	if (closeTargetThread)
		CloseHandle(timeout_targetThread);
	timeout_targetThread = NULL;
	testingFreeNsecString(timeoutstr);
	if (failNowOnError)
		testingTFailNow(t);
}

struct testingThread {
	uintptr_t handle;
	void (*f)(void *data);
	void *data;
};

static unsigned __stdcall threadThreadProc(void *data)
{
	testingThread *t = (testingThread *) data;

	(*(t->f))(t->data);
	return 0;
}

testingThread *testingNewThread(void (*f)(void *data), void *data)
{
	testingThread *t;
	HRESULT hr;

	t = testingprivNew(testingThread);
	t->f = f;
	t->data = data;

	hr = hr_beginthreadex(NULL, 0, threadThreadProc, t, 0, NULL, &(t->handle));
	if (hr != S_OK)
		testingprivInternalError("error creating thread: 0x%08I32X", hr);
	return t;
}

void testingThreadWaitAndFree(testingThread *t)
{
	HRESULT hr;

	hr = hrWaitForSingleObject((HANDLE) (t->handle), INFINITE);
	if (hr != S_OK)
		testingprivInternalError("error waiting for thread to finish: 0x%08I32X", hr);
	CloseHandle((HANDLE) (t->handle));
	testingprivFree(t);
}
