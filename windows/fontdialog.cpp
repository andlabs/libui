// 14 april 2016
#include "uipriv_windows.h"

struct fontDialog {
	HWND hwnd;
	HWND familyCombobox;
	HWND styleCombobox;
	HWND sizeCombobox;
	HWND smallCapsCheckbox;
	// TODO desc;
	fontCollection *fc;
	IDWriteFontFamily **families;
	UINT32 nFamilies;
};

static LRESULT cbAddString(HWND cb, WCHAR *str)
{
	LRESULT lr;

	lr = SendMessageW(cb, CB_ADDSTRING, 0, (LPARAM) str);
	if (lr == (LRESULT) CB_ERR || lr == (LRESULT) CB_ERRSPACE)
		logLastError("error adding item to combobox in cbAddString()");
	return lr;
}

static LRESULT cbInsertStringAtTop(HWND cb, WCHAR *str)
{
	LRESULT lr;

	lr = SendMessageW(cb, CB_INSERTSTRING, 0, (LPARAM) str);
	if (lr == (LRESULT) CB_ERR || lr == (LRESULT) CB_ERRSPACE)
		logLastError("error inserting item to combobox in cbInsertStringAtTop()");
	return lr;
}

static struct fontDialog *beginFontDialog(HWND hwnd, LPARAM lParam)
{
	struct fontDialog *f;
	UINT32 i;
	WCHAR *wname;
	LRESULT ten;
	HRESULT hr;

	f = uiNew(struct fontDialog);
	f->hwnd = hwnd;

	f->familyCombobox = GetDlgItem(f->hwnd, rcFontFamilyCombobox);
	if (f->familyCombobox == NULL)
		logLastError("error getting font family combobox handle in beginFontDialog()");
	f->styleCombobox = GetDlgItem(f->hwnd, rcFontStyleCombobox);
	if (f->styleCombobox == NULL)
		logLastError("error getting font style combobox handle in beginFontDialog()");
	f->sizeCombobox = GetDlgItem(f->hwnd, rcFontSizeCombobox);
	if (f->sizeCombobox == NULL)
		logLastError("error getting font size combobox handle in beginFontDialog()");
	f->smallCapsCheckbox = GetDlgItem(f->hwnd, rcFontSmallCapsCheckbox);
	if (f->smallCapsCheckbox == NULL)
		logLastError("error getting small caps checkbox handle in beginFontDialog()");

	f->fc = loadFontCollection();
	f->nFamilies = f->fc->fonts->GetFontFamilyCount();
	f->families = new IDWriteFontFamily *[f->nFamilies];
	for (i = 0; i < f->nFamilies; i++) {
		hr = f->fc->fonts->GetFontFamily(i, &(f->families[i]));
		if (hr != S_OK)
			logHRESULT("error getting font family in beginFontDialog()", hr);
		wname = fontCollectionFamilyName(f->fc, f->families[i]);
		cbAddString(f->familyCombobox, wname);
		uiFree(wname);
	}

	// TODO all comboboxes should select on type; these already scroll on type but not select

	// TODO behavior for the real thing:
	// - if prior size is in list, select and scroll to it
	// - if not, select nothing and don't scroll list at all (keep at top)
	// we do 8 and 9 later
	ten = cbAddString(f->sizeCombobox, L"10");
	cbAddString(f->sizeCombobox, L"11");
	cbAddString(f->sizeCombobox, L"12");
	cbAddString(f->sizeCombobox, L"14");
	cbAddString(f->sizeCombobox, L"16");
	cbAddString(f->sizeCombobox, L"18");
	cbAddString(f->sizeCombobox, L"20");
	cbAddString(f->sizeCombobox, L"22");
	cbAddString(f->sizeCombobox, L"24");
	cbAddString(f->sizeCombobox, L"26");
	cbAddString(f->sizeCombobox, L"28");
	cbAddString(f->sizeCombobox, L"36");
	cbAddString(f->sizeCombobox, L"48");
	cbAddString(f->sizeCombobox, L"72");
	if (SendMessageW(f->sizeCombobox, CB_SETCURSEL, (WPARAM) ten, 0) != ten)
		logLastError("error selecting 10 in the size combobox in beginFontDialog()");
	// if we just use CB_ADDSTRING 8 and 9 will appear at the bottom of the list due to lexicographical sorting
	// if we use CB_INSERTSTRING instead it won't
	cbInsertStringAtTop(f->sizeCombobox, L"9");
	cbInsertStringAtTop(f->sizeCombobox, L"8");
	// 10 moved because of the above; figure out where it is now
	// we selected it earlier; getting the selection is easiest
	ten = SendMessageW(f->sizeCombobox, CB_GETCURSEL, 0, 0);
	// and finally put 10 at the top to imitate ChooseFont()
	if (SendMessageW(f->sizeCombobox, CB_SETTOPINDEX, (WPARAM) ten, 0) != 0)
		logLastError("error making 10 visible in the size combobox in beginFontDialog()");

	// TODO use GetComboBoxInfo() to add ES_NUMBER to the size combobox's edit box

	return f;
}

static void endFontDialog(struct fontDialog *f, INT_PTR code)
{
	UINT32 i;

	for (i = 0; i < f->nFamilies; i++)
		f->families[i]->Release();
	delete[] f->families;
	fontCollectionFree(f->fc);
	if (EndDialog(f->hwnd, code) == 0)
		logLastError("error ending font dialog in endFontDialog()");
	uiFree(f);
}

static INT_PTR tryFinishDialog(struct fontDialog *f, WPARAM wParam)
{
	// cancelling
	if (LOWORD(wParam) != IDOK) {
		endFontDialog(f, 1);
		return TRUE;
	}

	// TODO

	endFontDialog(f, 2);
	return TRUE;
}

static INT_PTR CALLBACK fontDialogDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct fontDialog *f;

	f = (struct fontDialog *) GetWindowLongPtrW(hwnd, DWLP_USER);
	if (f == NULL) {
		if (uMsg == WM_INITDIALOG) {
			f = beginFontDialog(hwnd, lParam);
			SetWindowLongPtrW(hwnd, DWLP_USER, (LONG_PTR) f);
			return TRUE;
		}
		return FALSE;
	}

	switch (uMsg) {
	case WM_COMMAND:
		if (HIWORD(wParam) != BN_CLICKED)
			return FALSE;
		return tryFinishDialog(f, wParam);
	}
	return FALSE;
}

void showFontDialog(HWND parent)
{
	switch (DialogBoxParamW(hInstance, MAKEINTRESOURCE(rcFontDialog), parent, fontDialogDlgProc, (LPARAM) NULL)) {
	case 1:
		// TODO cancel
		break;
	case 2:
		// TODO OK
		break;
	default:
		logLastError("error running font dialog in showFontDialog()");
	}
}
