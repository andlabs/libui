// 17 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

// TODOs
// - clicking on the same item restarts editing instead of cancels it

// this is not how the real list view positions and sizes the edit control, but this is a) close enough b) a lot easier to follow c) something I can actually get working d) something I'm slightly more comfortable including in libui
static HRESULT resizeEdit(uiTable *t, WCHAR *wstr, int iItem, int iSubItem)
{
	uiprivTableMetrics *m;
	RECT r;
	HDC dc;
	HFONT prevFont;
	TEXTMETRICW tm;
	SIZE textSize;
	RECT editRect, clientRect;
	HRESULT hr;

	hr = uiprivTableGetMetrics(t, iItem, iSubItem, &m);
	if (hr != S_OK)
		return hr;
	r = m->realTextRect;
	uiprivFree(m);

	// TODO check errors for all these
	dc = GetDC(t->hwnd);		// use the list view DC since we're using its coordinate space
	prevFont = (HFONT) SelectObject(dc, hMessageFont);
	GetTextMetricsW(dc, &tm);
	GetTextExtentPoint32W(dc, wstr, wcslen(wstr), &textSize);
	SelectObject(dc, prevFont);
	ReleaseDC(t->hwnd, dc);

	SendMessageW(t->edit, EM_GETRECT, 0, (LPARAM) (&editRect));
	r.left -= editRect.left;
	// find the top of the text
	r.top += ((r.bottom - r.top) - tm.tmHeight) / 2;
	// and move THAT by the right offset
	r.top -= editRect.top;
	r.right = r.left + textSize.cx;
	// the real listview does this to add some extra space at the end
	// TODO this still isn't enough space
	r.right += 4 * GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CYEDGE);
	// and make the bottom equally positioned to the top
	r.bottom = r.top + editRect.top + tm.tmHeight + editRect.top;

	// make sure the edit box doesn't stretch outside the listview
	// the list view just does this, which is dumb for when the list view wouldn't be visible at all, but given that it doesn't scroll the edit into view either...
	// TODO check errors
	GetClientRect(t->hwnd, &clientRect);
	IntersectRect(&r, &r, &clientRect);

	// TODO check error or use the right function
	SetWindowPos(t->edit, NULL,
		r.left, r.top,
		r.right - r.left, r.bottom - r.top,
		SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
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
	uiTableValue *value;
	WCHAR *wstr;
	HRESULT hr;

	// the real list view accepts changes to the existing item when editing a new item
	hr = uiprivTableFinishEditingText(t);
	if (hr != S_OK)
		return hr;

	// the real list view creates the edit control with the string
	value = uiprivTableModelCellValue(t->model, iItem, p->textModelColumn);
	wstr = toUTF16(uiTableValueString(value));
	uiFreeTableValue(value);
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
	// TODO check errors
	SetWindowSubclass(t->edit, editSubProc, 0, (DWORD_PTR) t);

	hr = resizeEdit(t, wstr, iItem, iSubItem);
	if (hr != S_OK)
		// TODO proper cleanup
		return hr;
	// TODO check errors on these two, if any
	SetFocus(t->edit);
	ShowWindow(t->edit, SW_SHOW);
	SendMessageW(t->edit, EM_SETSEL, 0, (LPARAM) (-1));

	uiprivFree(wstr);
	t->editedItem = iItem;
	t->editedSubitem = iSubItem;
	return S_OK;
}

HRESULT uiprivTableResizeWhileEditing(uiTable *t)
{
	WCHAR *text;
	HRESULT hr;

	if (t->edit == NULL)
		return S_OK;
	text = windowText(t->edit);
	hr = resizeEdit(t, text, t->editedItem, t->editedSubitem);
	uiprivFree(text);
	return hr;
}

HRESULT uiprivTableFinishEditingText(uiTable *t)
{
	uiprivTableColumnParams *p;
	uiTableValue *value;
	char *text;

	if (t->edit == NULL)
		return S_OK;
	text = uiWindowsWindowText(t->edit);
	value = uiNewTableValueString(text);
	uiFreeText(text);
	p = (*(t->columns))[t->editedSubitem];
	uiprivTableModelSetCellValue(t->model, t->editedItem, p->textModelColumn, value);
	uiFreeTableValue(value);
	// always refresh the value in case the model rejected it
	if (SendMessageW(t->hwnd, LVM_UPDATE, (WPARAM) (t->editedItem), 0) == (LRESULT) (-1)) {
		logLastError(L"LVM_UPDATE");
		return E_FAIL;
	}
	return uiprivTableAbortEditingText(t);
}

HRESULT uiprivTableAbortEditingText(uiTable *t)
{
	HWND edit;

	if (t->edit == NULL)
		return S_OK;
	// set t->edit to NULL now so we don't trigger commits on focus killed
	edit = t->edit;
	t->edit = NULL;

	if (DestroyWindow(edit) == 0) {
		logLastError(L"DestroyWindow()");
		return E_FAIL;
	}
	return S_OK;
}

HRESULT uiprivTableHandleNM_CLICK(uiTable *t, NMITEMACTIVATE *nm, LRESULT *lResult)
{
	LVHITTESTINFO ht;
	uiprivTableColumnParams *p;
	int modelColumn, editableColumn;
	bool text, checkbox;
	uiTableValue *value;
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
		editableColumn = p->textEditableModelColumn;
		text = true;
	} else if (p->checkboxModelColumn != -1) {
		modelColumn = p->checkboxModelColumn;
		editableColumn = p->checkboxEditableModelColumn;
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

	if (!uiprivTableModelCellEditable(t->model, ht.iItem, editableColumn))
		goto done;

	if (text) {
		hr = openEditControl(t, ht.iItem, ht.iSubItem, p);
		if (hr != S_OK)
			return hr;
	} else if (checkbox) {
		if ((ht.flags & LVHT_ONITEMICON) == 0)
			goto done;
		value = uiprivTableModelCellValue(t->model, ht.iItem, modelColumn);
		checked = uiTableValueInt(value);
		uiFreeTableValue(value);
		value = uiNewTableValueInt(!checked);
		uiprivTableModelSetCellValue(t->model, ht.iItem, modelColumn, value);
		uiFreeTableValue(value);
	} else
		uiprivTableModelSetCellValue(t->model, ht.iItem, modelColumn, NULL);
	// always refresh the value in case the model rejected it
	if (SendMessageW(t->hwnd, LVM_UPDATE, (WPARAM) (ht.iItem), 0) == (LRESULT) (-1)) {
		logLastError(L"LVM_UPDATE");
		return E_FAIL;
	}

done:
	*lResult = 0;
	return S_OK;
}
