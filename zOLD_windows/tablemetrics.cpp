// 14 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

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

HRESULT uiprivTableGetMetrics(uiTable *t, int iItem, int iSubItem, uiprivTableMetrics **mout)
{
	uiprivTableMetrics *m;
	uiprivTableColumnParams *p;
	LRESULT state;
	HWND header;
	RECT r;
	HRESULT hr;

	if (mout == NULL)
		return E_POINTER;

	m = uiprivNew(uiprivTableMetrics);

	p = (*(t->columns))[iSubItem];
	m->hasText = p->textModelColumn != -1;
	m->hasImage = (p->imageModelColumn != -1) || (p->checkboxModelColumn != -1);
	state = SendMessageW(t->hwnd, LVM_GETITEMSTATE, iItem, LVIS_FOCUSED | LVIS_SELECTED);
	m->focused = (state & LVIS_FOCUSED) != 0;
	m->selected = (state & LVIS_SELECTED) != 0;

	// TODO check LRESULT bad parameters here
	hr = itemRect(S_OK, t, LVM_GETITEMRECT, iItem,
		LVIR_BOUNDS, 0, FALSE, &(m->itemBounds));
	hr = itemRect(hr, t, LVM_GETITEMRECT, iItem,
		LVIR_ICON, 0, FALSE, &(m->itemIcon));
	hr = itemRect(hr, t, LVM_GETITEMRECT, iItem,
		LVIR_LABEL, 0, FALSE, &(m->itemLabel));
	hr = itemRect(hr, t, LVM_GETSUBITEMRECT, iItem,
		LVIR_BOUNDS, iSubItem, 0, &(m->subitemBounds));
	hr = itemRect(hr, t, LVM_GETSUBITEMRECT, iItem,
		LVIR_ICON, iSubItem, 0, &(m->subitemIcon));
	if (hr != S_OK)
		goto fail;
	// LVM_GETSUBITEMRECT treats LVIR_LABEL as the same as
	// LVIR_BOUNDS, so we can't use that directly. Instead, let's
	// assume the text is immediately after the icon. The correct
	// rect will be determined by
	// computeOtherRectsAndDrawBackgrounds() above.
	m->subitemLabel = m->subitemBounds;
	m->subitemLabel.left = m->subitemIcon.right;
	// And on iSubItem == 0, LVIF_GETSUBITEMRECT still includes
	// all the subitems, which we don't want.
	if (iSubItem == 0) {
		m->subitemBounds.right = m->itemLabel.right;
		m->subitemLabel.right = m->itemLabel.right;
	}

	header = (HWND) SendMessageW(t->hwnd, LVM_GETHEADER, 0, 0);
	m->bitmapMargin = SendMessageW(header, HDM_GETBITMAPMARGIN, 0, 0);
	if (ImageList_GetIconSize(t->imagelist, &(m->cxIcon), &(m->cyIcon)) == 0) {
		logLastError(L"ImageList_GetIconSize()");
		hr = E_FAIL;
		goto fail;
	}

	r = m->subitemLabel;
	if (!m->hasText && !m->hasImage)
		r = m->subitemBounds;
	else if (!m->hasImage && iSubItem != 0)
		// By default, this will include images; we're not drawing
		// images, so we will manually draw over the image area.
		// There's a second part to this; see below.
		r.left = m->subitemBounds.left;
	m->realTextBackground = r;

	m->realTextRect = r;
	// TODO confirm whether this really happens on column 0 as well
	if (m->hasImage && iSubItem != 0)
		// Normally there's this many hard-coded logical units
		// of blank space, followed by the background, followed
		// by a bitmap margin's worth of space. This looks bad,
		// so we overrule that to start the background immediately
		// and the text after the hard-coded amount.
		m->realTextRect.left += 2;
	else if (iSubItem != 0)
		// In the case of subitem text without an image, we draw
		// text one bitmap margin away from the left edge.
		m->realTextRect.left += m->bitmapMargin;

	*mout = m;
	return S_OK;
fail:
	uiprivFree(m);
	*mout = NULL;
	return hr;
}
