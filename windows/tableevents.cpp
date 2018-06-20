// 17 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

// TODO deduplicate this with tabledraw.cpp
static HRESULT itemRect(HRESULT hr, uiTable *t, UINT uMsg, WPARAM wParam, LONG left, LONG top, LRESULT bad, RECT *r)
{
	if (hr != S_OK)
		return hr;
	ZeroMemory(r, sizeof (RECT));
	r->left = left;
	r->top = top;
	if (SendMessageW(t->hwnd, uMsg, wParam, (LPARAM) r) == bad) {
		logLastError(L"itemRect() message");
		return E_FAIL;
	}
	return S_OK;
}

// the real list view intercepts these keys to control editing
static LRESULT CALLBACK editSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIDSubclass, DWORD_PTR dwRefData)
{
	uiTable *t = (uiTable *) dwRefData;
	HRESULT hr;

	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
		// TODO handle VK_TAB and VK_SHIFT+VK_TAB
		case VK_RETURN:
			hr = uiprivTableFinishEditingText(t);
			if (hr != S_OK) {
				// TODO
			}
			return 0;		// yes, the real list view just returns here
		case VK_ESCAPE:
			hr = uiprivTableAbortEditingText(t);
			if (hr != S_OK) {
				// TODO
			}
			return 0;
		}
		break;
	// the real list view also forces these flags
	case WM_GETDLGCODE:
		return DLGC_HASSETSEL | DLGC_WANTALLKEYS;
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, editSubProc, uIDSubclass) == FALSE)
			logLastError(L"RemoveWindowSubclass()");
		// fall through
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

static HRESULT openEditControl(uiTable *t, int iItem, int iSubItem, uiprivTableColumnParams *p)
{
	RECT itemLabel;
	RECT subitemBounds, subitemIcon, subitemLabel;
	uiTableData *data;
	WCHAR *wstr;
	RECT r;
	LONG xInflate, yInflate;
	HRESULT hr;

	// the real list view accepts changes to the existing item when editing a new item
	hr = uiprivTableFinishEditingText(t);
	if (hr != S_OK)
		return hr;

	// compute this in advance so we don't have to needlessly call DestroyWindow() later
	// TODO deduplicate this code with tabledraw.cpp
	// TODO check LRESULT bad parameters here
	hr = itemRect(S_OK, t, LVM_GETITEMRECT, iItem,
		LVIR_LABEL, 0, FALSE, &itemLabel);
	hr = itemRect(hr, t, LVM_GETSUBITEMRECT, iItem,
		LVIR_BOUNDS, iSubItem, 0, &subitemBounds);
	hr = itemRect(hr, t, LVM_GETSUBITEMRECT, iItem,
		LVIR_ICON, iSubItem, 0, &subitemIcon);
	if (hr != S_OK)
		return hr;
	// LVM_GETSUBITEMRECT treats LVIR_LABEL as the same as
	// LVIR_BOUNDS, so we can't use that directly. Instead, let's
	// assume the text is immediately after the icon. The correct
	// rect will be determined by
	// computeOtherRectsAndDrawBackgrounds() above.
	subitemLabel = subitemBounds;
	subitemLabel.left = subitemIcon.right;
	// And on iSubItem == 0, LVIF_GETSUBITEMRECT still includes
	// all the subitems, which we don't want.
	if (iSubItem == 0) {
		subitemBounds.right = itemLabel.right;
		subitemLabel.right = itemLabel.right;
	}
	if ((p->imageModelColumn == -1 && p->checkboxModelColumn == -1) && iSubItem != 0)
		// By default, this will include images; we're not drawing
		// images, so we will manually draw over the image area.
		// There's a second part to this; see below.
		subitemLabel.left = subitemBounds.left;
	if ((p->imageModelColumn != -1 || p->checkboxModelColumn != -1) && iSubItem != 0)
		// Normally there's this many hard-coded logical units
		// of blank space, followed by the background, followed
		// by a bitmap margin's worth of space. This looks bad,
		// so we overrule that to start the background immediately
		// and the text after the hard-coded amount.
		subitemLabel.left += 2;
	else if (iSubItem != 0) {
		HWND header;

		// In the case of subitem text without an image, we draw
		// text one bitmap margin away from the left edge.
		header = (HWND) SendMessageW(t->hwnd, LVM_GETHEADER, 0, 0);
		subitemLabel.left += SendMessageW(header, HDM_GETBITMAPMARGIN, 0, 0);
	}

	// the real list view creates the edit control with the string
	data = (*(t->model->mh->CellValue))(t->model->mh, t->model, iItem, p->textModelColumn);
	wstr = toUTF16(uiTableDataString(data));
	uiFreeTableData(data);
	// TODO copy WS_EX_RTLREADING
	t->edit = CreateWindowExW(0,
		L"EDIT", wstr,
		// these styles are what the normal listview edit uses
		WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER | ES_AUTOHSCROLL,
		// as is this size
		0, 0, 16384, 16384,
		// and this control ID
		t->hwnd, (HMENU) 1, hInstance, NULL);
	if (t->edit == NULL) {
		logLastError(L"CreateWindowExW()");
		uiprivFree(wstr);
		return E_FAIL;
	}
	SendMessageW(t->edit, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	// this is not how the real list view positions and sizes the edit control, but this is a) close enough b) a lot easier to follow c) something I can actually get working d) something I'm slightly more comfortable including in libui
	{
		HDC dc;
		HFONT prevFont;
		TEXTMETRICW tm;
		SIZE textSize;
		RECT editRect;

		// TODO deduplicate this with tabledraw.cpp
		// TODO check errors for all these
		dc = GetDC(t->hwnd);		// yes, real list view uses itself here
		prevFont = (HFONT) SelectObject(dc, hMessageFont);
		GetTextMetricsW(dc, &tm);
		GetTextExtentPoint32W(dc, wstr, wcslen(wstr), &textSize);
		SelectObject(dc, prevFont);
		ReleaseDC(t->hwnd, dc);

		SendMessageW(t->edit, EM_GETRECT, 0, (LPARAM) (&editRect));
		r.left = subitemLabel.left - editRect.left;
		// find the top of the text
		r.top = subitemLabel.top + ((subitemLabel.bottom - subitemLabel.top) - tm.tmHeight) / 2;
		// and move THAT by the right offset
		r.top = r.top - editRect.top;
		r.right = subitemLabel.left + textSize.cx;
		// the real listview does this to add some extra space at the end
		// TODO this still isn't enough space
		r.right += 4 * GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CYEDGE);
		// and make the bottom equally positioned to the top
		r.bottom = r.top + editRect.top + tm.tmHeight + editRect.top;
	}

	// TODO check error or use the right function
	SetWindowPos(t->edit, NULL,
		r.left, r.top,
		r.right - r.left, r.bottom - r.top,
		SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	// TODO get the correct constant from the real list view
	ShowWindow(t->edit, SW_SHOWDEFAULT);

	uiprivFree(wstr);
	t->editedItem = iItem;
	t->editedSubitem = iSubItem;
	return S_OK;
}

HRESULT uiprivTableFinishEditingText(uiTable *t)
{
	if (t->edit == NULL)
		return S_OK;
	return uiprivTableAbortEditingText(t);
}

HRESULT uiprivTableAbortEditingText(uiTable *t)
{
	if (t->edit == NULL)
		return S_OK;
	if (DestroyWindow(t->edit) == 0) {
		logLastError(L"DestroyWindow()");
		return E_FAIL;
	}
	t->edit = NULL;
	return S_OK;
}

HRESULT uiprivTableHandleNM_CLICK(uiTable *t, NMITEMACTIVATE *nm, LRESULT *lResult)
{
	LVHITTESTINFO ht;
	uiprivTableColumnParams *p;
	int modelColumn, editableColumn;
	bool text, checkbox;
	uiTableData *data;
	int checked, editable;
	HRESULT hr;

	ZeroMemory(&ht, sizeof (LVHITTESTINFO));
	ht.pt = nm->ptAction;
	if (SendMessageW(t->hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM) (&ht)) == (LRESULT) (-1))
		goto done;

	modelColumn = -1;
	editableColumn = -1;
	text = false;
	checkbox = false;
	p = (*(t->columns))[ht.iSubItem];
	if (p->textModelColumn != -1) {
		modelColumn = p->textModelColumn;
		editableColumn = p->textEditableColumn;
		text = true;
	} else if (p->checkboxModelColumn != -1) {
		modelColumn = p->checkboxModelColumn;
		editableColumn = p->checkboxEditableColumn;
		checkbox = true;
	} else if (p->buttonModelColumn != -1) {
		modelColumn = p->buttonModelColumn;
		editableColumn = p->buttonClickableModelColumn;
	}
	if (modelColumn == -1)
		goto done;

	if (text && t->inDoubleClickTimer)
		// don't even ask for info if it's too soon to edit text
		goto done;

	switch (editableColumn) {
	case uiTableModelColumnNeverEditable:
		goto done;
	case uiTableModelColumnAlwaysEditable:
		break;
	default:
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, ht.iItem, editableColumn);
		editable = uiTableDataInt(data);
		uiFreeTableData(data);
		if (!editable)
			goto done;
	}

	if (text) {
		hr = openEditControl(t, ht.iItem, ht.iSubItem, p);
		if (hr != S_OK)
			return hr;
	} else if (checkbox) {
		if ((ht.flags & LVHT_ONITEMICON) == 0)
			goto done;
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, ht.iItem, modelColumn);
		checked = uiTableDataInt(data);
		uiFreeTableData(data);
		data = uiNewTableDataInt(!checked);
		(*(t->model->mh->SetCellValue))(t->model->mh, t->model, ht.iItem, modelColumn, data);
		uiFreeTableData(data);
	} else
		(*(t->model->mh->SetCellValue))(t->model->mh, t->model, ht.iItem, modelColumn, NULL);
	// always refresh the value in case the model rejected it
	if (SendMessageW(t->hwnd, LVM_UPDATE, (WPARAM) (ht.iItem), 0) == (LRESULT) (-1)) {
		logLastError(L"LVM_UPDATE");
		return E_FAIL;
	}

done:
	*lResult = 0;
	return S_OK;
}
