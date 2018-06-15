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
