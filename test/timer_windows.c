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
