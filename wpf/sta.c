// 25 november 2015
#include "../windows/winapi.h"
#include "../ui.h"

// If we don't set up the current thread otherwise, the first time .net tries to call out to unmanaged code, it will automatically set up a MTA for COM.
// This is not what we want; we need a STA instead.
// Since we're not in control of main(), we can't stick a [STAThread] on it, so we have to do it ourselves.
// This is a separate .c file for two reasons:
// 1) To avoid the unmanaged jump that a call to CoInitialize() would do (it seems to detect a call to CoInitialize()/CoInitializeEx() but let's not rely on it)
// 2) To avoid mixing Windows API headers with .net
// See also http://stackoverflow.com/questions/24348205/how-do-i-solve-this-com-issue-in-c

extern void initWPF(void);
//extern void uninitWPF(void);

void wpfInit(void)
{
	HRESULT hr;

	// TODO https://msdn.microsoft.com/en-us/library/5s8ee185%28v=vs.71%29.aspx use CoInitializeEx()?
	hr = CoInitialize(NULL);
	if (hr != S_OK && hr != S_FALSE)
		DebugBreak();

	// now do the rest of initialization on the managed side
	initWPF();
}

/*TODO
void uiUninit(void)
{
	uninitWPF();
	CoUninitialize();
}
*/
