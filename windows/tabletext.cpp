// 13 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

// This file handles text in tables.

HRESULT uiprivLVN_GETDISPINFOText(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p)
{
	uiTableData *data;
	WCHAR *wstr;
	HRESULT hr;

	if ((nm->item.mask & LVIF_TEXT) == 0)
		return S_OK;
	if (p->textModelColumn != -1)
		return S_OK;

	data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->item.iItem, p->textModelColumn);
	wstr = toUTF16(uiTableDataString(data));
	uiFreeTableData(data);
	// we could just make pszText into a freshly allocated conversion and avoid the limitation of cchTextMax
	// but then we would have to keep things around for some amount of time (some pages on MSDN say 2 additional LVN_GETDISPINFO messages)
	// and in practice, anything that results in extra LVN_GETDISPINFO messages (such as fillSubitemDrawParams() below) will break this counting
	// TODO make it so we don't have to make a copy; instead we can convert directly into pszText (this will also avoid the risk of having a dangling surrogate pair at the end)
	wcsncpy(nm->item.pszText, wstr, nm->item.cchTextMax);
	nm->item.pszText[nm->item.cchTextMax - 1] = L'\0';
	uiprivFree(wstr);
	return S_OK;
}

HRESULT uiprivNM_CUSTOMDRAWText(uiTable *t, NMLVCUSTOMDRAW *nm, uiprivTableColumnParams *p, uiprivSubitemDrawParams *dp)
{
	COLORREF color;
	COLORREF prev;
	int prevMode;
	RECT r;
	uiTableData *data;
	WCHAR *wstr;

	if (p->textModelColumn == -1)
		return S_OK;

	if (dp->selected)
		color = GetSysColor(COLOR_HIGHLIGHTTEXT);
	else if (p->textParams.ColorModelColumn != -1)
		color = uiprivTableBlendedColorFromModel(t, nm, p->textParams.ColorModelColumn, COLOR_WINDOWTEXT);
	else
		color = GetSysColor(COLOR_WINDOWTEXT);
	prev = SetTextColor(nm->nmcd.hdc, color);
	if (prev == CLR_INVALID) {
		logLastError(L"SetTextColor()");
		return E_FAIL;
	}
	prevMode = SetBkMode(nm->nmcd.hdc, TRANSPARENT);
	if (prevMode == 0) {
		logLastError(L"SetBkMode()");
		return E_FAIL;
	}

	r = dp->label;
	if (p->imageModelColumn != -1 || p->checkboxModelColumn != -1)
		// text is actually drawn two logical units to the right of the beginning of the text rect
		// TODO confirm this for the first column on both image and imageless cases
		// TODO actually this whole thing is wrong for imageless columns
		r.left += 2;
	else
		r.left = dp->bounds.left + dp->bitmapMargin;

	data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->nmcd.dwItemSpec, p->textModelColumn);
	wstr = toUTF16(uiTableDataString(data));
	uiFreeTableData(data);
	// these flags are a menagerie of flags from various sources: guessing, the Windows 2000 source leak, various custom draw examples on the web, etc.
	// TODO find the real correct flags
	if (DrawTextW(nm->nmcd.hdc, wstr, -1, &r, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX | DT_EDITCONTROL) == 0) {
		uiprivFree(wstr);
		logLastError(L"DrawTextW()");
		return E_FAIL;
	}
	uiprivFree(wstr);

	// TODO decide once and for all what to compare to here and with SelectObject()
	if (SetBkMode(nm->nmcd.hdc, prevMode) != TRANSPARENT) {
		logLastError(L"SetBkMode() prev");
		return E_FAIL;
	}
	if (SetTextColor(nm->nmcd.hdc, prev) != color) {
		logLastError(L"SetTextColor() prev");
		return E_FAIL;
	}
	return S_OK;
}
