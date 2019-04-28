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

struct testingTimer {
	LARGE_INTEGER start;
	LARGE_INTEGER end;
};

testingTimer *testingNewTimer(void)
{
	testingTimer *t;

	t = (testingTimer *) malloc(sizeof (testingTimer));
	// TODO handle failure
	memset(t, 0, sizeof (testingTimer));
	return t;
}

void testingFreeTimer(testingTimer *t)
{
	free(t);
}

void testingTimerStart(testingTimer *t)
{
	QueryPerformanceCounter(&(t->start));
}

void testingTimerEnd(testingTimer *t)
{
	QueryPerformanceCounter(&(t->end));
}

int64_t testingTimerNsec(testingTimer *t)
{
	LARGE_INTEGER qpf;
	int64_t c;
	int64_t sec;
	int64_t subsec;
	double subsecf;

	QueryPerformanceFrequency(&qpf);
	c = t->end.QuadPart - t->start.QuadPart;

	sec = c / qpf.QuadPart;
	sec *= testingNsecPerSec;

	subsec = c % qpf.QuadPart;
	subsecf = (double) subsec;
	subsecf /= qpf.QuadPart;
	subsecf *= testingNsecPerSec;
	subsec = (int64_t) subsecf;

	return sec + subsec;
}

static jmp_buf timeout_ret;

static void onTimeout(void)
{
	longjmp(timeout_ret, 1);
}

static HANDLE timeout_timer;
static HANDLE timeout_finished;
static HANDLE timeout_targetThread;
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
#elif defined(_X86_)
	ctx->Eip = (DWORD) onTimeout;
#elif defined(_IA64_)
	// TODO verify that this is correct
	ctx->StIIP = (ULONGLONG) onTimeout;
// TODO arm64 (I need to get the headers for this)
#else
#error unknown CPU architecture; cannot create CONTEXT objects for CPU-specific Windows test code
#endif
}

static unsigned __stdcall timerThreadProc(void *data)
{
	HANDLE objects[2];
	CONTEXT ctx;
	DWORD ret;
	DWORD lastError;

	objects[0] = timeout_timer;
	objects[1] = timeout_finished;
	timeout_hr = S_OK;
	SetLastError(0);
	ret = WaitForMultipleObjectsEx(2, objects,
		FALSE, INFINITE, FALSE);
	if (ret == WAIT_FAILED) {
		lastError = GetLastError();
		timeout_hr = E_FAIL;
		if (lastError != 0)
			timeout_hr = HRESULT_FROM_WIN32(lastError);
		ret = WAIT_OBJECT_0;
	}
	if (ret == WAIT_OBJECT_0 + 1)
		// we succeeded; do nothing
		return 0;

	// we timed out (or there was an error); signal it
	// TODO check errors
	SuspendThread(timeout_targetThread);
	setContextForGet(&ctx);
	GetThreadContext(timeout_targetThread, &ctx);
	setContextForSet(&ctx);
	SetThreadContext(timeout_targetThread, &ctx);
	// and force the thread to return from GetMessage(), if we are indeed in that
	PostThreadMessage(GetThreadId(timeout_targetThread), WM_NULL, 0, 0);
	ResumeThread(timeout_targetThread);
	return 0;
}

void testingprivRunWithTimeout(testingT *t, const char *file, long line, int64_t timeout, void (*f)(testingT *t, void *data), void *data, const char *comment, int failNowOnError)
{
	char *timeoutstr;
	int closeTargetThread = 0;
	uintptr_t timerThread = 0;
	LARGE_INTEGER timer;
	BOOL ret;
	DWORD lastError;
	HRESULT hr;

	timeoutstr = testingNsecString(timeout);

	SetLastError(0);
	ret = DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &timeout_targetThread,
		0, FALSE, DUPLICATE_SAME_ACCESS);
	if (ret == 0) {
		lastError = GetLastError();
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error getting current thread for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}
	closeTargetThread = 1;

	SetLastError(0);
	timeout_timer = CreateWaitableTimerW(NULL, TRUE, NULL);
	if (timeout_timer == NULL) {
		lastError = GetLastError();
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error creating timer for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	SetLastError(0);
	timeout_finished = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (timeout_finished == NULL) {
		lastError = GetLastError();
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error creating finished event for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	SetLastError(0);
	timer.QuadPart = timeout / 100;
	timer.QuadPart = -timer.QuadPart;
	ret = SetWaitableTimer(timeout_timer, &timer, 0, NULL, NULL, FALSE);
	if (ret == 0) {
		lastError = (DWORD) _doserrno;
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error applying %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	// _doserrno is the equivalent of GetLastError(), or at least that's how _beginthreadex() uses it.
	_doserrno = 0;
	// don't start the thread until after we call setjmp()
	timerThread = _beginthreadex(NULL, 0, timerThreadProc, NULL, CREATE_SUSPENDED, NULL);
	if (timerThread == 0) {
		lastError = (DWORD) _doserrno;
		hr = E_FAIL;
		if (lastError != 0)
			hr = HRESULT_FROM_WIN32(lastError);
		testingprivTLogfFull(t, file, line, "error creating timer thread for %s timeout: 0x%08I32X", comment, hr);
		testingTFail(t);
		goto out;
	}

	if (setjmp(timeout_ret) == 0) {
		// TODO check error
		ResumeThread((HANDLE) timerThread);
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
		// TODO check errors
		SetEvent(timeout_finished);
		WaitForSingleObject((HANDLE) timerThread, INFINITE);
		// TODO end check errors
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
