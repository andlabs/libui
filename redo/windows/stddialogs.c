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
	// TODO OFN_SHAREAWARE?
	// better question: TODO keep networking?
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_FORCESHOWHIDDEN | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS | OFN_NOTESTFILECREATE | OFN_PATHMUSTEXIST;
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
	// TODO OFN_SHAREAWARE?
	// better question: TODO keep networking?
	// TODO OFN_PATHMUSTEXIST?
	ofn.Flags = OFN_EXPLORER | OFN_FORCESHOWHIDDEN | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS | OFN_NOTESTFILECREATE | OFN_OVERWRITEPROMPT;
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
