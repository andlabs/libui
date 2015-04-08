// 17 july 2014
#include "uipriv_windows.h"

static ULONG_PTR comctlManifestCookie;
static HMODULE comctl32;

// these are listed as WINAPI in both Microsoft's and MinGW's headers, but not on MSDN for some reason
BOOL (*WINAPI fv_SetWindowSubclass)(HWND, SUBCLASSPROC, UINT_PTR, DWORD_PTR);
BOOL (*WINAPI fv_RemoveWindowSubclass)(HWND, SUBCLASSPROC, UINT_PTR);
LRESULT (*WINAPI fv_DefSubclassProc)(HWND, UINT, WPARAM, LPARAM);

#define wantedICCClasses ( \
	ICC_STANDARD_CLASSES |	/* user32.dll controls */	\
	ICC_PROGRESS_CLASS |		/* progress bars */		\
	ICC_TAB_CLASSES |			/* tabs */				\
	ICC_LISTVIEW_CLASSES |		/* table headers */		\
	ICC_UPDOWN_CLASS |		/* spinboxes */		\
	0)

// note that this is an 8-bit character string we're writing; see the encoding clause
static const char manifest[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\n<assemblyIdentity\n    version=\"1.0.0.0\"\n    processorArchitecture=\"*\"\n    name=\"CompanyName.ProductName.YourApplication\"\n    type=\"win32\"\n/>\n<description>Your application description here.</description>\n<dependency>\n    <dependentAssembly>\n        <assemblyIdentity\n            type=\"win32\"\n            name=\"Microsoft.Windows.Common-Controls\"\n            version=\"6.0.0.0\"\n            processorArchitecture=\"*\"\n            publicKeyToken=\"6595b64144ccf1df\"\n            language=\"*\"\n        />\n    </dependentAssembly>\n</dependency>\n</assembly>\n";

/*
Windows requires a manifest file to enable Common Controls version 6.
The only way to not require an external manifest is to synthesize the manifest ourselves.
We can use the activation context API to load it at runtime.
References:
- http://stackoverflow.com/questions/4308503/how-to-enable-visual-styles-without-a-manifest
- http://support.microsoft.com/kb/830033
Because neither Go nor MinGW have ways to compile in resources like this (as far as I know), we have to do the work ourselves.
*/
const char *initCommonControls(void)
{
	WCHAR temppath[MAX_PATH + 1];
	WCHAR filename[MAX_PATH + 1];
	HANDLE file;
	DWORD nExpected, nGot;
	ACTCTX actctx;
	HANDLE ac;
	INITCOMMONCONTROLSEX icc;
	FARPROC f;
	// this is listed as WINAPI in both Microsoft's and MinGW's headers, but not on MSDN for some reason
	BOOL (*WINAPI ficc)(const LPINITCOMMONCONTROLSEX);

	if (GetTempPathW(MAX_PATH + 1, temppath) == 0)
		return "getting temporary path for writing manifest file in initCommonControls()";
	if (GetTempFileNameW(temppath, L"manifest", 0, filename) == 0)
		return "getting temporary filename for writing manifest file in initCommonControls()";
	file = CreateFileW(filename, GENERIC_WRITE,
		0,			// don't share while writing
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == NULL)
		return "creating manifest file in initCommonControls()";
	nExpected = (sizeof manifest / sizeof manifest[0]) - 1;		// - 1 to omit the terminating null character)
	SetLastError(0);		// catch errorless short writes
	if (WriteFile(file, manifest, nExpected, &nGot, NULL) == 0)
		return "writing manifest file in initCommonControls()";
	if (nGot != nExpected) {
		DWORD lasterr;

		lasterr = GetLastError();
		if (lasterr == 0)
			return "writing entire manifest file (short write) without error code in initCommonControls()";
		return "writing entire manifest file (short write) in initCommonControls()";
	}
	if (CloseHandle(file) == 0)
		return "closing manifest file (this IS an error here because not doing so will prevent Windows from being able to use the manifest file in an activation context) in initCommonControls()";

	ZeroMemory(&actctx, sizeof (ACTCTX));
	actctx.cbSize = sizeof (ACTCTX);
	actctx.dwFlags = ACTCTX_FLAG_SET_PROCESS_DEFAULT;
	actctx.lpSource = filename;
	ac = CreateActCtx(&actctx);
	if (ac == INVALID_HANDLE_VALUE)
		return "creating activation context for synthesized manifest file in initCommonControls()";
	if (ActivateActCtx(ac, &comctlManifestCookie) == FALSE)
		return "activating activation context for synthesized manifest file in initCommonControls()";

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = wantedICCClasses;

	comctl32 = LoadLibraryW(L"comctl32.dll");
	if (comctl32 == NULL)
		return "loading comctl32.dll in initCommonControls()";

	// GetProcAddress() only takes a multibyte string
#define LOAD(fn) f = GetProcAddress(comctl32, fn); \
	if (f == NULL) \
		return "loading " fn "() in initCommonControls()";

	LOAD("InitCommonControlsEx");
	ficc = (BOOL (*WINAPI)(const LPINITCOMMONCONTROLSEX)) f;
	LOAD("SetWindowSubclass");
	fv_SetWindowSubclass = (BOOL (*WINAPI)(HWND, SUBCLASSPROC, UINT_PTR, DWORD_PTR)) f;
	LOAD("RemoveWindowSubclass");
	fv_RemoveWindowSubclass = (BOOL (*WINAPI)(HWND, SUBCLASSPROC, UINT_PTR)) f;
	LOAD("DefSubclassProc");
	fv_DefSubclassProc = (LRESULT (*WINAPI)(HWND, UINT, WPARAM, LPARAM)) f;

	if ((*ficc)(&icc) == FALSE)
		return "initializing Common Controls (comctl32.dll) in initCommonControls()";

	return NULL;
}
