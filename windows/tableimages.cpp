// 10 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

/*
This file handles both images and checkboxes in tables.

For images, we'll do a similar thing to what text columns do: cycle out images from the small image list every few LVN_GETDISPINFO notifications.

For checkboxes, the native list view checkbox functionality uses state images, but those are only supported on the main item, not on subitems. So instead, we'll do them on normal images instead.
TODO will this affect accessibility?

We'll use the small image list. For this, the first few items will be reserved for checkboxes, and the last few for cell images.
*/

#define nCheckboxImages 4

static HRESULT setCellImage(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p, uiTableData *data)
{
	int index;
	HDC dc;
	IWICImage *wb;
	HBITMAP b;

	index = t->smallIndex + nCheckboxImages;
	t->smallIndex++;
	t->smallIndex %= uiprivNumLVN_GETDISPINFOSkip;
	nm->item.iImage = index;

	dc = GetDC(t->hwnd);
	if (dc == NULL) {
		logLastError(L"GetDC()");
		return E_FAIL;
	}

	wb = uiprivImageAppropriateForDC(uiTableDataImage(data), dc);
	b = uiprivWICToGDI(wb, dc);
	if (ImageList_Replace(t->smallImages, index, b, NULL) == 0) {
		logLastError(L"ImageList_Replace()");
		return E_FAIL;
	}

	if (ReleaseDC(t->hwnd, dc) == 0) {
		logLastError(L"ReleaseDC()");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT uiprivLVN_GETDISPINFOImagesCheckboxes(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p)
{
	uiTableData *data;
	HRESULT hr;

	if ((nm->item.mask & LVIF_IMAGE) == 0)
		return S_OK;		// nothing to do here

	if (p->imageModelColumn != -1) {
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->item.iItem, p->imageModelColumn);
		hr = setCellImage(t, nm, p, data);
		uiFreeTableData(data);
		return hr;
	}

#if 0
	if (p->checkboxModelColumn != -1) {
		// TODO handle enabled
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->item.iItem, p->textModelColumn);
		checked = uiTableDataInt(data) != 0;
		uiFreeTableData(data);
		nm->item.iImage = 0;
		if (checked)
			nm->item.iImage = 1;
		nm->item.mask |= LVIF_IMAGE;
	}
#endif

	// if we got here, there's no image in this cell
	nm->item.iImage = 0;
	// having an image list always leaves space for an image on the main item :|
	// other places on the internet imply that you should be able to do this but that it shouldn't work
	// but it works perfectly (and pixel-perfectly too) for me, so...
	if (nm->item.iSubItem == 0) {
		nm->item.mask |= LVIF_INDENT;
		nm->item.iIndent = -1;
	}
	return S_OK;
}
