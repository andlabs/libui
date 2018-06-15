// 14 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

struct drawState {
	uiTable *t;
	uiTableModel *m;
	uiprivTableColumnParams *p;

	HDC dc;
	int iItem;
	int iSubItem;
	BOOL hasText;
	BOOL hasImage;
	BOOL selected;
	BOOL focused;

	RECT itemBounds;
	RECT itemIcon;
	RECT itemText;
	RECT subitemBounds;
	RECT subitemIcon;
	RECT subitemText;

	COLORREF bgColor;
	HBRUSH bgBrush;
	BOOL freeBgBrush;
	COLORREF textColor;
	HBRUSH textBrush;
	BOOL freeTextBrush;

	LRESULT bitmapMargins;
	int cxIcon;
	int cyIcon;

	RECT realTextRect;
	RECT focusRect;
};

static HRESULT computeAndDrawTextRect(struct drawState *s)
{
	RECT r;

	r = s->subitemText;
	if (!s->hasText && !s->hasImage)
		r = s->subitemBounds;

	if (FillRect(s->dc, &r, s->bgBrush) == 0) {
		logLastError(L"FillRect()");
		return E_FAIL;
	}
	UnionRect(&(s->focusRect), &(s->focusRect), &r);

	s->realTextRect = r;
	// TODO confirm whether this really happens on column 0 as well
	if (s->hasImage && s->iSubItem != 0)
		// Normally there's this many hard-coded logical units
		// of blank space, followed by the background, followed
		// by a bitmap margin's worth of space. This looks bad,
		// so we overrule that to start the background immediately
		// and the text after the hard-coded amount.
		s->realTextRect.left += 2;
	else if (s->iSubItem != 0)
		// In the case of subitem text without an image, we draw
		// text one bitmap margin away from the left edge.
		s->realTextRect.left += s->bitmapMargin;
	return S_OK;
}

static HRESULT drawTextPart(struct drawState *s)
{
	COLORREF prevText;
	int prevMode;
	RECT r;
	uiTableData *data;
	WCHAR *wstr;

	if (!s->hasText)
		return S_OK;

	prevText = SetTextColor(s->dc, s->textColor);
	if (prevText == CLR_INVALID) {
		logLastError(L"SetTextColor()");
		return E_FAIL;
	}
	prevMode = SetBkMode(s->dc, TRANSPARENT);
	if (prevMode == 0) {
		logLastError(L"SetBkMode()");
		return E_FAIL;
	}

	data = (*(s->m->mh->CellValue))(s->m->mh, s->m, s->iItem, s->p->textModelColumn);
	wstr = toUTF16(uiTableDataString(data));
	uiFreeTableData(data);
	// These flags are a menagerie of flags from various sources:
	// guessing, the Windows 2000 source leak, various custom
	// draw examples on the web, etc.
	// TODO find the real correct flags
	if (DrawTextW(s->dc, wstr, -1, &(s->realTextRect), DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX | DT_EDITCONTROL) == 0) {
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
	if (SetTextColor(nm->nmcd.hdc, prevText) != color) {
		logLastError(L"SetTextColor() prev");
		return E_FAIL;
	}
	return S_OK;
}

static HRESULT freeDrawState(struct drawState *s)
{
	HRESULT hr, hrret;

	hrret = S_OK;
	if (p->freeTextBrush) {
		if (DeleteObject(p->textBrush) == 0) {
			logLastError(L"DeleteObject()");
			hrret = E_FAIL;
			// continue cleaning up anyway
		}
		p->freeTextBrush = NO;
	}
	if (p->freeBgBrush) {
		if (DeleteObject(p->bgBrush) == 0) {
			logLastError(L"DeleteObject()");
			hrret = E_FAIL;
			// continue cleaning up anyway
		}
		p->freeBgBrush = NO;
	}
	return hrret;
}

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

static HRESULT fillDrawState(struct drawState *s, uiTable *t, NMLVCUSTOMDRAW *nm, uiprivTableColumnParams *p)
{
	LRESULT state;
	HWND header;
	HRESULT hr;

	ZeroMemory(s, sizeof (struct drawState));
	s->t = t;
	s->m = t->model;
	s->p = p;

	s->dc = nm->nmcd.hdc;
	s->iItem = nm->nmcd.dwItemSpec;
	s->iSubItem = nm->iSubItem;
	s->hasText = p->textModelColumn != -1;
	s->hasImage = (p->imageModelColumn != -1) || (p->checkboxModelColumn != -1);
	// nm->nmcd.uItemState CDIS_SELECTED is unreliable for the
	// listview configuration we have, so we must do this.
	state = SendMessageW(t->hwnd, LVM_GETITEMSTATE, nm->nmcd.dwItemSpec, LVIS_SELECTED);
	dp->selected = (state & LVIS_SELECTED) != 0;
	dp->focused = (nm->nmcd.uiTemState & CDIS_FOCUSED) != 0;

	hr = itemRect(S_OK, t, LVM_GETITEMRECT, LVIR_BOUNDS,
		0, &(s->itemBounds));
	hr = itemRect(hr, t, LVM_GETITEMRECT, LVIR_ICON,
		0, &(s->itemIcon));
	hr = itemRect(hr, t, LVM_GETITEMRECT, LVIR_LABEL,
		0, &(s->itemLabel));
	hr = itemRect(hr, t, LVM_GETSUBITEMRECT, LVIR_BOUNDS,
		s->iSubItem, &(s->subitemBounds));
	hr = itemRect(hr, t, LVM_GETSUBITEMRECT, LVIR_ICON,
		s->iSubItem, &(s->subitemIcon));
	if (hr != S_OK)
		goto fail;
	// LVM_GETSUBITEMRECT treats LVIR_LABEL as the same as
	// LVIR_BOUNDS, so we can't use that directly. Instead, let's
	// assume the text is immediately after the icon. The correct
	// rect will be determined by computeAndDrawTextRect()
	// above.
	s->subitemLabel = s->subitemBounds;
	s->subitemLabel.left = s->subitemIcon.right;

	if (s->selected) {
		s->bgColor = GetSysColor(COLOR_HIGHLIGHT);
		s->bgBrush = GetSysColorBrush(COLOR_HIGHLIGHT);
		s->textColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
		s->textBrush = GetSysColorBrush(COLOR_HIGHLIGHTTEXT);
	} else {
		uiTableData *data;

		s->bgColor = GetSysColor(COLOR_WINDOW);
		s->bgBrush = GetSysColorBrush(COLOR_WINDOW);
		if (t->backgroundColumn != -1) {
			data = (*(s->m->mh->CellValue))(s->m->mh, s->m, s->iItem, t->backgroundColumn);
			if (data != NULL) {
				uiTableDataColor(data, &r, &g, &b, &a);
				uiFreeTableData(data);
				s->bgColor = blend(s->bgColor, r, g, b, a);
				s->bgBrush = CreateSolidBrush(s->bgBrush);
				if (s->bgBrush == NULL) {
					logLastError(L"CreateSolidBrush()");
					hr = E_FAIL;
					goto fail;
				}
				s->freeBgBrush = TRUE;
			}
		}
		s->textColor = GetSysColor(COLOR_WINDOWTEXT);
		s->textBrush = GetSysColorBrush(COLOR_WINDOWTEXT);
		if (p->textParams.ColorModelColumn != -1) {
			data = (*(s->m->mh->CellValue))(s->m->mh, s->m, s->iItem, p->textParams.ColorModelColumn);
			if (data != NULL) {
				uiTableDataColor(data, &r, &g, &b, &a);
				uiFreeTableData(data);
				s->textColor = blend(s->bgColor, r, g, b, a);
				s->textBrush = CreateSolidBrush(s->textColor);
				if (s->textBrush == NULL) {
					logLastError(L"CreateSolidBrush()");
					hr = E_FAIL;
					goto fail;
				}
				s->freeTextBrush = TRUE;
			}
		}
	}

	header = (HWND) SendMessageW(t->hwnd, LVM_GETHEADER, 0, 0);
	s->bitmapMargin = SendMessageW(header, HDM_GETBITMAPMARGIN, 0, 0);
	// TODO

	return S_OK;
fail:
	// ignore the error; we need to return the one we got above
	freeDrawState(s);
	return hr;
}
