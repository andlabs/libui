// 22 may 2015
#include "uipriv_windows.h"

// note: FOS_SUPPORTSTREAMABLEITEMS doesn't seem to be supported on windows vista, or at least not with the flags we use

char *commonItemDialog(REFCLSID clsid, REFIID iid, FILEOPENDIALOGOPTIONS optsadd)
{
	IFileDialog *d;
	FILEOPENDIALOGOPTIONS opts;
	HWND dialogHelper;
	IShellItem *result;
	WCHAR *wname;
	char *name;
	HRESULT hr;

	hr = CoCreateInstance(clsid,
		NULL, CLSCTX_INPROC_SERVER,
		iid, (LPVOID *) (&d));
	if (hr != S_OK)
		logHRESULT("error creating common item dialog in commonItemDialog()", hr);
	hr = IFileDialog_GetOptions(d, &opts);
	if (hr != S_OK)
		logHRESULT("error getting current options in commonItemDialog()", hr);
	opts |= optsadd;
	hr = IFileDialog_SetOptions(d, opts);
	if (hr != S_OK)
		logHRESULT("error setting options in commonItemDialog()", hr);
	dialogHelper = beginDialogHelper();
	hr = IFileDialog_Show(d, dialogHelper);
	endDialogHelper(dialogHelper);
	if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		IFileDialog_Release(d);
		return NULL;
	}
	if (hr != S_OK)
		logHRESULT("error showing dialog in commonItemDialog()", hr);
	hr = IFileDialog_GetResult(d, &result);
	if (hr != S_OK)
		logHRESULT("error getting dialog result in commonItemDialog()", hr);
	hr = IShellItem_GetDisplayName(result, SIGDN_FILESYSPATH, &wname);
	if (hr != S_OK)
		logHRESULT("error getting filename in commonItemDialog()", hr);
	name = toUTF8(wname);
	CoTaskMemFree(wname);
	IShellItem_Release(result);
	IFileDialog_Release(d);
	return name;
}

char *uiOpenFile(void)
{
	return commonItemDialog(&CLSID_FileOpenDialog, &IID_IFileOpenDialog,
		FOS_NOCHANGEDIR | FOS_ALLNONSTORAGEITEMS | FOS_NOVALIDATE | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_SHAREAWARE | FOS_NOTESTFILECREATE | FOS_NODEREFERENCELINKS | FOS_FORCESHOWHIDDEN | FOS_DEFAULTNOMINIMODE);
}

char *uiSaveFile(void)
{
	return commonItemDialog(&CLSID_FileSaveDialog, &IID_IFileSaveDialog,
		// TODO strip FOS_NOREADONLYRETURN?
		FOS_OVERWRITEPROMPT | FOS_NOCHANGEDIR | FOS_ALLNONSTORAGEITEMS | FOS_NOVALIDATE | FOS_SHAREAWARE | FOS_NOTESTFILECREATE | FOS_NODEREFERENCELINKS | FOS_FORCESHOWHIDDEN | FOS_DEFAULTNOMINIMODE);
}

// TODO MinGW-w64 3.x doesn't support task dialogs
#define TDCBF_OK_BUTTON 0x0001
#define TD_ERROR_ICON MAKEINTRESOURCEW(-2)
typedef int TASKDIALOG_COMMON_BUTTON_FLAGS;           // Note: _TASKDIALOG_COMMON_BUTTON_FLAGS is an int
HRESULT (*WINAPI fv_TaskDialog)(_In_opt_ HWND hwndOwner, _In_opt_ HINSTANCE hInstance, _In_opt_ PCWSTR pszWindowTitle, _In_opt_ PCWSTR pszMainInstruction, _In_opt_ PCWSTR pszContent, TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons, _In_opt_ PCWSTR pszIcon, int *pnButton) = NULL;

static void msgbox(const char *title, const char *description, TASKDIALOG_COMMON_BUTTON_FLAGS buttons, PCWSTR icon)
{
	WCHAR *wtitle, *wdescription;
	HWND dialogHelper;
	HRESULT hr;

	if (fv_TaskDialog == NULL) {
		HANDLE h;

		h = LoadLibraryW(L"comctl32.dll");
		fv_TaskDialog = GetProcAddress(h, "TaskDialog");
	}

	wtitle = toUTF16(title);
	wdescription = toUTF16(description);

	dialogHelper = beginDialogHelper();
	hr = (*fv_TaskDialog)(dialogHelper, NULL, NULL, wtitle, wdescription, buttons, icon, NULL);
	if (hr != S_OK)
		logHRESULT("error showing task dialog in msgbox()", hr);
	endDialogHelper(dialogHelper);

	uiFree(wdescription);
	uiFree(wtitle);
}

void uiMsgBox(const char *title, const char *description)
{
	msgbox(title, description, TDCBF_OK_BUTTON, NULL);
}

void uiMsgBoxError(const char *title, const char *description)
{
	msgbox(title, description, TDCBF_OK_BUTTON, TD_ERROR_ICON);
}
