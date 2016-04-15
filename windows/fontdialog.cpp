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
};

static struct fontDialog *beginFontDialog(HWND hwnd, LPARAM lParam)
{
	struct fontDialog *f;

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

	return f;
}

static void endFontDialog(struct fontDialog *f, INT_PTR code)
{
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
