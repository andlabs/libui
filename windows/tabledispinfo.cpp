// 13 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

static HRESULT handleLVIF_TEXT(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p)
{
	uiTableData *data;
	WCHAR *wstr;
	int progress;
	HRESULT hr;

	if ((nm->item.mask & LVIF_TEXT) == 0)
		return S_OK;

	if (p->textModelColumn != -1) {
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->item.iItem, p->textModelColumn);
		wstr = toUTF16(uiTableDataString(data));
		uiFreeTableData(data);
		// We *could* just make pszText into a freshly allocated
		// conversion and avoid the limitation of cchTextMax.
		// But then, we would have to keep things around for some
		// amount of time (some pages on MSDN say 2 additional
		// LVN_GETDISPINFO messages). And in practice, anything
		// that results in extra LVN_GETDISPINFO messages (such
		// as LVN_GETITEMRECT with LVIR_LABEL) will break this
		// counting.
		// TODO make it so we don't have to make a copy; instead we can convert directly into pszText (this will also avoid the risk of having a dangling surrogate pair at the end)
		wcsncpy(nm->item.pszText, wstr, nm->item.cchTextMax);
		nm->item.pszText[nm->item.cchTextMax - 1] = L'\0';
		uiprivFree(wstr);
		return S_OK;
	}

	if (p->progressBarModelColumn != -1) {
		progress = uiprivTableProgress(t, nm->item.iItem, p->progressBarModelColumn, NULL);

		if (progress == -1) {
			// TODO either localize this or replace it with something that's language-neutral
			// TODO ensure null terminator
			wcsncpy(nm->item.pszText, L"Indeterminate", nm->item.cchTextMax);
			return S_OK;
		}
		// TODO ensure null terminator
		_snwprintf(nm->item.pszText, nm->item.cchTextMax, L"%d%%", progress);
		return S_OK;
	}

	return S_OK;
}

static HRESULT handleLVIF_IMAGE(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p)
{
	uiTableData *data;
	HRESULT hr;

	if (nm->item.iSubItem == 0 && p->imageModelColumn == -1 && p->checkboxModelColumn == -1) {
		// Having an image list always leaves space for an image
		// on the main item :|
		// Other places on the internet imply that you should be
		// able to do this but that it shouldn't work, but it works
		// perfectly (and pixel-perfectly too) for me, so...
		nm->item.mask |= LVIF_INDENT;
		nm->item.iIndent = -1;
	}
	if ((nm->item.mask & LVIF_IMAGE) == 0)
		return S_OK;		// nothing to do here

	// TODO see if the -1 part is correct
	// TODO see if we should use state instead of images for checkbox data
	nm->item.iImage = -1;
	if (p->imageModelColumn != -1 || p->checkboxModelColumn != -1)
		nm->item.iImage = 0;
	return S_OK;
}

HRESULT uiprivTableHandleLVN_GETDISPINFO(uiTable *t, NMLVDISPINFOW *nm, LRESULT *lResult)
{
	uiprivTableColumnParams *p;
	HRESULT hr;

	p = (*(t->columns))[nm->item.iSubItem];
	hr = handleLVIF_TEXT(t, nm, p);
	if (hr != S_OK)
		return hr;
	hr = handleLVIF_IMAGE(t, nm, p);
	if (hr != S_OK)
		return hr;
	*lResult = 0;
	return S_OK;
}
