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
#include "timer.h"
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

static HRESULT WINAPI hrWaitForSingleObject(HANDLE handle, DWORD timeout)
{
	DWORD ret;

	SetLastError(0);
	ret = WaitForSingleObject(handle, timeout);
	if (ret == WAIT_FAILED)
		return lastErrorToHRESULT();
	return S_OK;
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
