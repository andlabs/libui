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

static HRESULT openEditControl(uiTable *t, int iItem, int iSubItem, uiprivTableColumnParams *p)
{
	RECT itemLabel;
	RECT subitemBounds, subitemIcon, subitemLabel;
	uiTableData *data;
	WCHAR *wstr;
	RECT r;
	LONG xInflate, yInflate;
	HRESULT hr;

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
	uiprivFree(wstr);
	SendMessageW(t->edit, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	// and this is what the real list view does to size the edit control
	SendMessageW(t->edit, EM_GETRECT, 0, (LPARAM) (&r));
	xInflate = -(GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXBORDER));
	yInflate = -r.top;
	// TODO check error
	InflateRect(&subitemLabel, xInflate, yInflate);

	// TODO rewrite and integrate the variables of this; I'm not fully comfortable keeping it as it is
	// TODO check errors everywhere
	{
		HDC dc;
		HFONT prevFont;
		RECT textRect, editRect;
		int cxIconSpacing;
		LONG offsetY;

		// yes, the list view control uses the list view DC for this
		dc = GetDC(t->hwnd);
		prevFont = (HFONT) SelectObject(dc, hMessageFont);
		ZeroMemory(&textRect, sizeof (RECT));
		cxIconSpacing = GetSystemMetrics(SM_CXICONSPACING);
		textRect.right = cxIconSpacing - 2 * GetSystemMetrics(SM_CXEDGE);
		// yes, the real edit control uses DT_CENTER for some reason
		// TODO the real edit control filters out certain types of characters but I'm not sure what
		DrawTextW(dc, wstr, -1, &textRect, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE | DT_EDITCONTROL | DT_CALCRECT);
		ReleaseDC(t->hwnd, dc);
		if (textRect.right < cxIconSpacing / 4)
			textRect.right = cxIconSpacing / 4;
		offsetY = subitemLabel.top + ((textRect.bottom - textRect.top) - (subitemLabel.bottom - subitemLabel.top)) / 2;
		OffsetRect(&textRect, subitemLabel.left, offsetY);
		textRect.right += 4 * GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CYEDGE);
		SendMessageW(t->edit, EM_GETRECT, 0, (LPARAM) (&editRect));
		editRect.left = -editRect.left;
		editRect.top = -editRect.top;
		AdjustWindowRectEx(&editRect, getStyle(t->edit), FALSE, getExStyle(t->edit));
		r = textRect;
		InflateRect(&r, -editRect.left, -editRect.top);
	}

	// TODO check error or use the right function
	SetWindowPos(t->edit, NULL,
		r.left, r.top,
		r.right - r.left, r.bottom - r.top,
		SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	// TODO get the correct constant from the real list view
	ShowWindow(t->edit, SW_SHOWDEFAULT);

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
