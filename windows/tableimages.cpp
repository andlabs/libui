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
// - get rid of the extra bitmap margin space before text
// - get rid of extra whitespace before text on subitems

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

#define stateUnchecked 0
#define stateChecked 1
#define stateDisabled 2

static int checkboxIndex(uiTableModel *m, int row, int checkboxModelColumn, int checkboxEditableColumn)
{
	uiTableData *data;
	int ret;

	ret = stateUnchecked;
	data = (*(m->mh->CellValue))(m->mh, m, row, checkboxModelColumn);
	if (uiTableDataInt(data) != 0)
		ret = stateChecked;
	uiFreeTableData(data);

	switch (checkboxEditableColumn) {
	case uiTableModelColumnNeverEditable:
		ret += stateDisabled;
		break;
	case uiTableModelColumnAlwaysEditable:
		break;
	default:
		data = (*(m->mh->CellValue))(m->mh, m, row, checkboxEditableColumn);
		if (uiTableDataInt(data) != 0)
			ret += stateDisabled;
	}

	return ret;
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

// in order to properly look like checkboxes, we need to exclude them from being colored in by the selection rect
// however, there seems to be no way to do this natively, so we have to draw over ourselves (TODO?)
// hopefully the performance won't be too bad
// see also https://www.codeproject.com/Articles/79/Neat-Stuff-to-Do-in-List-Controls-Using-Custom-Dra
HRESULT uiprivNM_CUSTOMDRAWImagesCheckboxes(uiTable *t, NMLVCUSTOMDRAW *nm, LRESULT *lResult)
{
	uiprivTableColumnParams *p;
	int index;
	RECT r;

	if (nm->nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT)) {
		*lResult |= CDRF_NOTIFYPOSTPAINT;
		return S_OK;
	}
	if (nm->nmcd.dwDrawStage != (CDDS_SUBITEM | CDDS_ITEMPOSTPAINT))
		return S_OK;

	// only draw over checkboxes
	p = (*(t->columns))[nm->iSubItem];
	if (p->checkboxModelColumn == -1)
		return S_OK;

	index = checkboxIndex(t->model, nm->nmcd.dwItemSpec,
		p->checkboxModelColumn, p->checkboxEditableColumn);
	ZeroMemory(&r, sizeof (RECT));
	r.left = LVIR_ICON;
	r.top = nm->iSubItem;
	if (SendMessageW(t->hwnd, LVM_GETSUBITEMRECT, nm->nmcd.dwItemSpec, (LPARAM) (&r)) == 0) {
		logLastError(L"LVM_GETSUBITEMRECT");
		return E_FAIL;
	}
#if 0
	// TODO this is offset by one pixel on my system and everything I've found indicates this should not be happening???
	if (ImageList_Draw(t->smallImages, index, nm->nmcd.hdc,
		r.left, r.top, ILD_NORMAL) == 0) {
		logLastError(L"ImageList_Draw()");
		return E_FAIL;
	}
#endif
	return S_OK;
}

// references for checkbox drawing:
// - https://blogs.msdn.microsoft.com/oldnewthing/20171129-00/?p=97485
// - https://blogs.msdn.microsoft.com/oldnewthing/20171201-00/?p=97505

static UINT unthemedStates[] = {
	0,
	DFCS_CHECKED,
	DFCS_INACTIVE,
	DFCS_CHECKED | DFCS_INACTIVE,
};

static int themedStates[] = {
	CBS_UNCHECKEDNORMAL,
	CBS_CHECKEDNORMAL,
	CBS_UNCHECKEDDISABLED,
	CBS_CHECKEDDISABLED,
};

// TODO properly clean up on failure
static HRESULT mkCheckboxes(uiTable *t, HTHEME theme, HDC dc, int cxList, int cyList, int cxCheck, int cyCheck)
{
	BITMAPINFO bmi;
	HBITMAP b;
	VOID *bits;
	HDC cdc;
	HBITMAP prevBitmap;
	RECT r;
	int i;
	HRESULT hr;

	ZeroMemory(&bmi, sizeof (BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cxList * nCheckboxImages;
	bmi.bmiHeader.biHeight = cyList;
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
	// note that the actual list view does this only if unthemed, but it can get away with that since its image lists only contain checkmarks
	// ours don't, so we have to compromise until the above TODO is resolved so we don't draw alpha stuff on top of garbage
	if (theme == NULL || cxList != cxCheck || cyList != cyCheck) {
		r.left = 0;
		r.top = 0;
		r.right = cxList * nCheckboxImages;
		r.bottom = cyList;
		FillRect(cdc, &r, GetSysColorBrush(COLOR_WINDOW));
	}

	r.left = 0;
	r.top = 0;
	r.right = cxCheck;
	r.bottom = cyCheck;
	if (theme != NULL) {
		// because we're not making an image list exactly the correct size, we'll need to manually position the checkbox correctly
		// let's just center it for now
		// TODO make sure this is correct...
		r.left = (cxList - cxCheck) / 2;
		r.top = (cyList - cyCheck) / 2;
		r.right += r.left;
		r.bottom += r.top;
		for (i = 0; i < nCheckboxImages; i++) {
			hr = DrawThemeBackground(theme, cdc,
				BP_CHECKBOX, themedStates[i],
				&r, NULL);
			if (hr != S_OK) {
				logHRESULT(L"DrawThemeBackground()", hr);
				return hr;
			}
			r.left += cxList;
			r.right += cxList;
		}
	} else {
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
			r.left += cxList;
			r.right += cxList;
		}
	}

	if (SelectObject(cdc, prevBitmap) != ((HGDIOBJ) b)) {
		logLastError(L"SelectObject() prev");
		return E_FAIL;
	}
	if (DeleteDC(cdc) == 0) {
		logLastError(L"DeleteDC()");
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
	HDC dc;
	int cxList, cyList;
	HTHEME theme;
	SIZE sizeCheck;
	HRESULT hr;

	dc = GetDC(t->hwnd);
	if (dc == NULL) {
		logLastError(L"GetDC()");
		return E_FAIL;
	}

	cxList = GetSystemMetrics(SM_CXSMICON);
	cyList = GetSystemMetrics(SM_CYSMICON);
	sizeCheck.cx = cxList;
	sizeCheck.cy = cyList;
	theme = OpenThemeData(t->hwnd, L"button");
	if (theme != NULL) {
		hr = GetThemePartSize(theme, dc,
			BP_CHECKBOX, CBS_UNCHECKEDNORMAL,
			NULL, TS_DRAW, &sizeCheck);
		if (hr != S_OK) {
			logHRESULT(L"GetThemePartSize()", hr);
			return hr;			// TODO fall back?
		}
		// make sure these checkmarks fit
		// unthemed checkmarks will by the code above be smaller than cxList/cyList here
		if (cxList < sizeCheck.cx)
			cxList = sizeCheck.cx;
		if (cyList < sizeCheck.cy)
			cyList = sizeCheck.cy;
	}

	// TODO handle errors
	t->smallImages = ImageList_Create(cxList, cyList,
		ILC_COLOR32,
		nCheckboxImages + uiprivNumLVN_GETDISPINFOSkip, nCheckboxImages + uiprivNumLVN_GETDISPINFOSkip);
	if (t->smallImages == NULL) {
		logLastError(L"ImageList_Create()");
		return E_FAIL;
	}
	// TODO will this return NULL here because it's an initial state?
	SendMessageW(t->hwnd, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM) (t->smallImages));
	hr = mkCheckboxes(t, theme, dc, cxList, cyList, sizeCheck.cx, sizeCheck.cy);
	if (hr != S_OK)
		return hr;

	hr = CloseThemeData(theme);
	if (hr != S_OK) {
		logHRESULT(L"CloseThemeData()", hr);
		return hr;
	}
	if (ReleaseDC(t->hwnd, dc) == 0) {
		logLastError(L"ReleaseDC()");
		return E_FAIL;
	}
	return S_OK;
}
