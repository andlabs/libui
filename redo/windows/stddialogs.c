// 22 may 2015
#include "uipriv_windows.h"

// notes:
// - FOS_SUPPORTSTREAMABLEITEMS doesn't seem to be supported on windows vista, or at least not with the flags we use
// - even with FOS_NOVALIDATE the dialogs will reject invalid filenames (at least on Vista, anyway)

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

// TODO MinGW-w64 4.0.0 doesn't have the Task Dialog functions in its .lib files
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
