// 22 may 2015
#include "uipriv_windows.h"

// TODO when making Vista-only, use common item dialog

// this should be reasonable
#define NFILENAME 4096

char *uiOpenFile(void)
{
	WCHAR wfilename[NFILENAME];
	OPENFILENAMEW ofn;
	HWND dialogHelper;
	DWORD err;

	dialogHelper = beginDialogHelper();
	wfilename[0] = L'\0';			// required by GetOpenFileName() to indicate no previous filename
	ZeroMemory(&ofn, sizeof (OPENFILENAMEW));
	ofn.lStructSize = sizeof (OPENFILENAMEW);
	ofn.hwndOwner = dialogHelper;
	ofn.hInstance = hInstance;
	ofn.lpstrFilter = NULL;			// no filters
	ofn.lpstrFile = wfilename;
	ofn.nMaxFile = NFILENAME;		// seems to include null terminator according to docs
	ofn.lpstrInitialDir = NULL;			// let system decide
	ofn.lpstrTitle = NULL;			// let system decide
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_FORCESHOWHIDDEN | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS | OFN_NOTESTFILECREATE | OFN_PATHMUSTEXIST | OFN_SHAREAWARE;
	if (GetOpenFileNameW(&ofn) == FALSE) {
		err = CommDlgExtendedError();
		if (err != 0)
			// TODO
			complain("error running open file dialog", err);
		// otherwise user cancelled
		endDialogHelper(dialogHelper);
		return NULL;
	}
	endDialogHelper(dialogHelper);
	return toUTF8(wfilename);
}

char *uiSaveFile(void)
{
	WCHAR wfilename[NFILENAME];
	OPENFILENAMEW ofn;
	HWND dialogHelper;
	DWORD err;

	dialogHelper = beginDialogHelper();
	wfilename[0] = L'\0';			// required by GetOpenFileName() to indicate no previous filename
	ZeroMemory(&ofn, sizeof (OPENFILENAMEW));
	ofn.lStructSize = sizeof (OPENFILENAMEW);
	ofn.hwndOwner = dialogHelper;
	ofn.hInstance = hInstance;
	ofn.lpstrFilter = NULL;			// no filters
	ofn.lpstrFile = wfilename;
	ofn.nMaxFile = NFILENAME;		// seems to include null terminator according to docs
	ofn.lpstrInitialDir = NULL;			// let system decide
	ofn.lpstrTitle = NULL;			// let system decide
	// TODO OFN_PATHMUSTEXIST?
	ofn.Flags = OFN_EXPLORER | OFN_FORCESHOWHIDDEN | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS | OFN_NOTESTFILECREATE | OFN_OVERWRITEPROMPT | OFN_SHAREAWARE;
	if (GetSaveFileNameW(&ofn) == FALSE) {
		err = CommDlgExtendedError();
		if (err != 0)
			// TODO
			complain("error running open file dialog", err);
		// otherwise user cancelled
		endDialogHelper(dialogHelper);
		return NULL;
	}
	endDialogHelper(dialogHelper);
	return toUTF8(wfilename);
}

// TODO MinGW-w64 doesn't support task dialogs
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
