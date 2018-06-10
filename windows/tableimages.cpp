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

// checkboxes TODOs:
// - see if we need to get rid of the extra margin in subitems
// - see if we need to get rid of the glow effect

#define nCheckboxImages 4

static HRESULT setCellImage(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p, uiTableData *data)
{
	int index;
	HDC dc;
	IWICBitmap *wb;
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
	// TODO rewrite this condition to make more sense; possibly swap the if and else blocks too
	if (ImageList_GetImageCount(t->smallImages) > index) {
		if (ImageList_Replace(t->smallImages, index, b, NULL) == 0) {
			logLastError(L"ImageList_Replace()");
			return E_FAIL;
		}
	} else
		if (ImageList_Add(t->smallImages, b, NULL) == -1) {
			logLastError(L"ImageList_Add()");
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
		// TODO we actually need to do the first column fix here too...
		return S_OK;		// nothing to do here

	if (p->imageModelColumn != -1) {
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->item.iItem, p->imageModelColumn);
		hr = setCellImage(t, nm, p, data);
		uiFreeTableData(data);
		return hr;
	}

	if (p->checkboxModelColumn != -1) {
#if 0
		// TODO handle enabled
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->item.iItem, p->textModelColumn);
		checked = uiTableDataInt(data) != 0;
		uiFreeTableData(data);
		nm->item.iImage = 0;
		if (checked)
			nm->item.iImage = 1;
		nm->item.mask |= LVIF_IMAGE;
#endif
		nm->item.mask |= LVIF_IMAGE;
		nm->item.iImage = nm->item.iItem % 4;
		return S_OK;
	}

	// if we got here, there's no image in this cell
	nm->item.mask &= ~LVIF_IMAGE;
	// having an image list always leaves space for an image on the main item :|
	// other places on the internet imply that you should be able to do this but that it shouldn't work
	// but it works perfectly (and pixel-perfectly too) for me, so...
	// TODO it doesn't work anymore...
	if (nm->item.iSubItem == 0) {
		nm->item.mask |= LVIF_INDENT;
		nm->item.iIndent = -1;
	}
	return S_OK;
}

// see https://blogs.msdn.microsoft.com/oldnewthing/20171129-00/?p=97485
static UINT unthemedStates[] = {
	0,
	DFCS_CHECKED,
	DFCS_INACTIVE,
	DFCS_CHECKED | DFCS_INACTIVE,
};

// TODO call this whenever either theme, system colors (maybe? TODO), or DPI change
// TODO properly clean up on failure
static HRESULT mkCheckboxesUnthemed(uiTable *t, int cx, int cy)
{
	HDC dc;
	BITMAPINFO bmi;
	HBITMAP b;
	VOID *bits;
	HDC cdc;
	HBITMAP prevBitmap;
	RECT r;
	int i;

	dc = GetDC(t->hwnd);
	if (dc == NULL) {
		logLastError(L"GetDC()");
		return E_FAIL;
	}
	ZeroMemory(&bmi, sizeof (BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cx * nCheckboxImages;
	bmi.bmiHeader.biHeight = cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	b = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS,
		&bits, NULL, 0);
	if (b == NULL) {
		logLastError(L"CreateDIBSection()");
		return E_FAIL;
	}

	cdc = CreateCompatibleDC(dc);
	if (cdc == NULL) {
		logLastError(L"CreateCompatibleDC()");
		return E_FAIL;
	}
	prevBitmap = (HBITMAP) SelectObject(cdc, b);
	if (prevBitmap == NULL) {
		logLastError(L"SelectObject() b");
		return E_FAIL;
	}

	// the actual list view LVS_EX_CHECKBOXES code does this to ensure the entire image is valid, not just the parts that are drawn after resizing
	// TODO find a better, alpha-friendly way to do this
	r.left = 0;
	r.top = 0;
	r.right = cx * nCheckboxImages;
	r.bottom = cy;
	FillRect(cdc, &r, GetSysColorBrush(COLOR_WINDOW));

	r.left = 0;
	r.top = 0;
	r.right = cx;
	r.bottom = cy;
	// this is what the actual list view LVS_EX_CHECKBOXES code does to more correctly size the checkboxes
	// TODO check errors
	InflateRect(&r, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));
	r.right++;
	r.bottom++;
	for (i = 0; i < nCheckboxImages; i++) {
		if (DrawFrameControl(cdc, &r,
			DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT | unthemedStates[i]) == 0) {
			logLastError(L"DrawFrameControl()");
			return E_FAIL;
		}
		r.left += cx;
		r.right += cx;
	}

	if (SelectObject(cdc, prevBitmap) != ((HGDIOBJ) b)) {
		logLastError(L"SelectObject() prev");
		return E_FAIL;
	}
	if (DeleteDC(cdc) == 0) {
		logLastError(L"DeleteDC()");
		return E_FAIL;
	}
	if (ReleaseDC(t->hwnd, dc) == 0) {
		logLastError(L"ReleaseDC()");
		return E_FAIL;
	}

	if (ImageList_Add(t->smallImages, b, NULL) == -1) {
		logLastError(L"ImageList_Add()");
		return E_FAIL;
	}

	// TODO error check
	DeleteObject(b);
	return S_OK;
}

// TODO run again when the DPI changes
HRESULT uiprivTableSetupImagesCheckboxes(uiTable *t)
{
	int cx, cy;

	cx = GetSystemMetrics(SM_CXSMICON);
	cy = GetSystemMetrics(SM_CYSMICON);
	// TODO handle errors
	t->smallImages = ImageList_Create(cx, cy,
		ILC_COLOR32,
		nCheckboxImages + uiprivNumLVN_GETDISPINFOSkip, nCheckboxImages + uiprivNumLVN_GETDISPINFOSkip);
	if (t->smallImages == NULL) {
		logLastError(L"ImageList_Create()");
		return E_FAIL;
	}
	// TODO will this return NULL here because it's an initial state?
	SendMessageW(t->hwnd, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM) (t->smallImages));
	return mkCheckboxesUnthemed(t, cx, cy);
}
