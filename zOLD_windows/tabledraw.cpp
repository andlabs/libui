// 14 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

// TODOs:
// - properly hide selection when not focused (or switch on LVS_SHOWSELALWAYS and draw that state)

// TODO maybe split this into item and subitem structs?
struct drawState {
	uiTable *t;
	uiTableModel *model;
	uiprivTableColumnParams *p;

	HDC dc;
	int iItem;
	int iSubItem;

	uiprivTableMetrics *m;

	COLORREF bgColor;
	HBRUSH bgBrush;
	BOOL freeBgBrush;
	COLORREF textColor;
	HBRUSH textBrush;
	BOOL freeTextBrush;
};

static HRESULT drawBackgrounds(HRESULT hr, struct drawState *s)
{
	if (hr != S_OK)
		return hr;
	if (s->m->hasImage)
		if (FillRect(s->dc, &(s->m->subitemIcon), GetSysColorBrush(COLOR_WINDOW)) == 0) {
			logLastError(L"FillRect() icon");
			return E_FAIL;
		}
	if (FillRect(s->dc, &(s->m->realTextBackground), s->bgBrush) == 0) {
		logLastError(L"FillRect()");
		return E_FAIL;
	}
	return S_OK;
}

static void centerImageRect(RECT *image, RECT *space)
{
	LONG xoff, yoff;

	// first make sure both have the same upper-left
	xoff = image->left - space->left;
	yoff = image->top - space->top;
	image->left -= xoff;
	image->top -= yoff;
	image->right -= xoff;
	image->bottom -= yoff;

	// now center
	xoff = ((space->right - space->left) - (image->right - image->left)) / 2;
	yoff = ((space->bottom - space->top) - (image->bottom - image->top)) / 2;
	image->left += xoff;
	image->top += yoff;
	image->right += xoff;
	image->bottom += yoff;
}

static HRESULT drawImagePart(HRESULT hr, struct drawState *s)
{
	uiTableValue *value;
	IWICBitmap *wb;
	HBITMAP b;
	RECT r;
	UINT fStyle;

	if (hr != S_OK)
		return hr;
	if (s->p->imageModelColumn == -1)
		return S_OK;

	value = uiprivTableModelCellValue(s->model, s->iItem, s->p->imageModelColumn);
	wb = uiprivImageAppropriateForDC(uiTableValueImage(value), s->dc);
	uiFreeTableValue(value);

	hr = uiprivWICToGDI(wb, s->dc, s->m->cxIcon, s->m->cyIcon, &b);
	if (hr != S_OK)
		return hr;
	// TODO rewrite this condition to make more sense; possibly swap the if and else blocks too
	// TODO proper cleanup
	if (ImageList_GetImageCount(s->t->imagelist) > 1) {
		if (ImageList_Replace(s->t->imagelist, 0, b, NULL) == 0) {
			logLastError(L"ImageList_Replace()");
			return E_FAIL;
		}
	} else
		if (ImageList_Add(s->t->imagelist, b, NULL) == -1) {
			logLastError(L"ImageList_Add()");
			return E_FAIL;
		}
	// TODO error check
	DeleteObject(b);

	r = s->m->subitemIcon;
	r.right = r.left + s->m->cxIcon;
	r.bottom = r.top + s->m->cyIcon;
	centerImageRect(&r, &(s->m->subitemIcon));
	fStyle = ILD_NORMAL;
	if (s->m->selected)
		fStyle = ILD_SELECTED;
	if (ImageList_Draw(s->t->imagelist, 0,
		s->dc, r.left, r.top, fStyle) == 0) {
		logLastError(L"ImageList_Draw()");
		return E_FAIL;
	}
	return S_OK;
}

// references for checkbox drawing:
// - https://blogs.msdn.microsoft.com/oldnewthing/20171129-00/?p=97485
// - https://blogs.msdn.microsoft.com/oldnewthing/20171201-00/?p=97505

static HRESULT drawUnthemedCheckbox(struct drawState *s, int checked, int enabled)
{
	RECT r;
	UINT state;

	r = s->m->subitemIcon;
	// this is what the actual list view LVS_EX_CHECKBOXES code does to size the checkboxes
	// TODO reverify the initial size
	r.right = r.left + GetSystemMetrics(SM_CXSMICON);
	r.bottom = r.top + GetSystemMetrics(SM_CYSMICON);
	if (InflateRect(&r, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE)) == 0) {
		logLastError(L"InflateRect()");
		return E_FAIL;
	}
	r.right++;
	r.bottom++;

	centerImageRect(&r, &(s->m->subitemIcon));
	state = DFCS_BUTTONCHECK | DFCS_FLAT;
	if (checked)
		state |= DFCS_CHECKED;
	if (!enabled)
		state |= DFCS_INACTIVE;
	if (DrawFrameControl(s->dc, &r, DFC_BUTTON, state) == 0) {
		logLastError(L"DrawFrameControl()");
		return E_FAIL;
	}
	return S_OK;
}

static HRESULT drawThemedCheckbox(struct drawState *s, HTHEME theme, int checked, int enabled)
{
	RECT r;
	SIZE size;
	int state;
	HRESULT hr;

	hr = GetThemePartSize(theme, s->dc,
		BP_CHECKBOX, CBS_UNCHECKEDNORMAL,
		NULL, TS_DRAW, &size);
	if (hr != S_OK) {
		logHRESULT(L"GetThemePartSize()", hr);
		return hr;			// TODO fall back?
	}
	r = s->m->subitemIcon;
	r.right = r.left + size.cx;
	r.bottom = r.top + size.cy;

	centerImageRect(&r, &(s->m->subitemIcon));
	if (!checked && enabled)
		state = CBS_UNCHECKEDNORMAL;
	else if (checked && enabled)
		state = CBS_CHECKEDNORMAL;
	else if (!checked && !enabled)
		state = CBS_UNCHECKEDDISABLED;
	else
		state = CBS_CHECKEDDISABLED;
	hr = DrawThemeBackground(theme, s->dc,
		BP_CHECKBOX, state,
		&r, NULL);
	if (hr != S_OK) {
		logHRESULT(L"DrawThemeBackground()", hr);
		return hr;
	}
	return S_OK;
}

static HRESULT drawCheckboxPart(HRESULT hr, struct drawState *s)
{
	uiTableValue *value;
	int checked, enabled;
	HTHEME theme;

	if (hr != S_OK)
		return hr;
	if (s->p->checkboxModelColumn == -1)
		return S_OK;

	value = uiprivTableModelCellValue(s->model, s->iItem, s->p->checkboxModelColumn);
	checked = uiTableValueInt(value);
	uiFreeTableValue(value);
	enabled = uiprivTableModelCellEditable(s->model, s->iItem, s->p->checkboxEditableModelColumn);

	theme = OpenThemeData(s->t->hwnd, L"button");
	if (theme != NULL) {
		hr = drawThemedCheckbox(s, theme, checked, enabled);
		if (hr != S_OK)
			return hr;
		hr = CloseThemeData(theme);
		if (hr != S_OK) {
			logHRESULT(L"CloseThemeData()", hr);
			return hr;
		}
	} else {
		hr = drawUnthemedCheckbox(s, checked, enabled);
		if (hr != S_OK)
			return hr;
	}
	return S_OK;
}

static HRESULT drawTextPart(HRESULT hr, struct drawState *s)
{
	COLORREF prevText;
	int prevMode;
	RECT r;
	uiTableValue *value;
	WCHAR *wstr;

	if (hr != S_OK)
		return hr;
	if (!s->m->hasText)
		return S_OK;
	// don't draw the text underneath an edit control
	if (s->t->edit != NULL && s->t->editedItem == s->iItem && s->t->editedSubitem == s->iSubItem)
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

	value = uiprivTableModelCellValue(s->model, s->iItem, s->p->textModelColumn);
	wstr = toUTF16(uiTableValueString(value));
	uiFreeTableValue(value);
	// These flags are a menagerie of flags from various sources:
	// guessing, the Windows 2000 source leak, various custom
	// draw examples on the web, etc.
	// TODO find the real correct flags
	if (DrawTextW(s->dc, wstr, -1, &(s->m->realTextRect), DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX | DT_EDITCONTROL) == 0) {
		uiprivFree(wstr);
		logLastError(L"DrawTextW()");
		return E_FAIL;
	}
	uiprivFree(wstr);

	// TODO decide once and for all what to compare to here and with SelectObject()
	if (SetBkMode(s->dc, prevMode) != TRANSPARENT) {
		logLastError(L"SetBkMode() prev");
		return E_FAIL;
	}
	if (SetTextColor(s->dc, prevText) != s->textColor) {
		logLastError(L"SetTextColor() prev");
		return E_FAIL;
	}
	return S_OK;
}

// much of this is to imitate what shell32.dll's CDrawProgressBar does
#define indeterminateSegments 8

static HRESULT drawProgressBarPart(HRESULT hr, struct drawState *s)
{
	int progress;
	LONG indeterminatePos;
	HTHEME theme;
	RECT r;
	RECT rBorder, rFill[2];
	int i, nFill;
	TEXTMETRICW tm;
	int sysColor;

	if (hr != S_OK)
		return hr;
	if (s->p->progressBarModelColumn == -1)
		return S_OK;

	progress = uiprivTableProgress(s->t, s->iItem, s->iSubItem, s->p->progressBarModelColumn, &indeterminatePos);

	theme = OpenThemeData(s->t->hwnd, L"PROGRESS");

	if (GetTextMetricsW(s->dc, &tm) == 0) {
		logLastError(L"GetTextMetricsW()");
		hr = E_FAIL;
		goto fail;
	}
	r = s->m->subitemBounds;
	// this sets the height of the progressbar and vertically centers it in one fell swoop
	r.top += (r.bottom - tm.tmHeight - r.top) / 2;
	r.bottom = r.top + tm.tmHeight;

	// TODO check errors
	rBorder = r;
	InflateRect(&rBorder, -1, -1);
	if (theme != NULL) {
		RECT crect;

		hr = GetThemeBackgroundContentRect(theme, s->dc,
			PP_TRANSPARENTBAR, PBBS_NORMAL,
			&rBorder, &crect);
		if (hr != S_OK) {
			logHRESULT(L"GetThemeBackgroundContentRect()", hr);
			goto fail;
		}
		hr = DrawThemeBackground(theme, s->dc,
			PP_TRANSPARENTBAR, PBBS_NORMAL,
			&crect, NULL);
		if (hr != S_OK) {
			logHRESULT(L"DrawThemeBackground() border", hr);
			goto fail;
		}
	} else {
		HPEN pen, prevPen;
		HBRUSH brush, prevBrush;

		sysColor = COLOR_HIGHLIGHT;
		if (s->m->selected)
			sysColor = COLOR_HIGHLIGHTTEXT;

		// TODO check errors everywhere
		pen = CreatePen(PS_SOLID, 1, GetSysColor(sysColor));
		prevPen = (HPEN) SelectObject(s->dc, pen);
		brush = (HBRUSH) GetStockObject(NULL_BRUSH);
		prevBrush = (HBRUSH) SelectObject(s->dc, brush);
		Rectangle(s->dc, rBorder.left, rBorder.top, rBorder.right, rBorder.bottom);
		SelectObject(s->dc, prevBrush);
		SelectObject(s->dc, prevPen);
		DeleteObject(pen);
	}

	nFill = 1;
	rFill[0] = r;
	// TODO check error
	InflateRect(&rFill[0], -1, -1);
	if (progress != -1)
		rFill[0].right -= (rFill[0].right - rFill[0].left) * (100 - progress) / 100;
	else {
		LONG barWidth;
		LONG pieceWidth;

		// TODO explain all this
		// TODO this should really start the progressbar scrolling into view instead of already on screen when first set
		rFill[1] = rFill[0];		// save in case we need it
		barWidth = rFill[0].right - rFill[0].left;
		pieceWidth = barWidth / indeterminateSegments;
		rFill[0].left += indeterminatePos % barWidth;
		if ((rFill[0].left + pieceWidth) >= rFill[0].right) {
			// make this piece wrap back around
			nFill++;
			rFill[1].right = rFill[1].left + (pieceWidth - (rFill[0].right - rFill[0].left));
		} else
			rFill[0].right = rFill[0].left + pieceWidth;
	}
	for (i = 0; i < nFill; i++)
		if (theme != NULL) {
			hr = DrawThemeBackground(theme, s->dc,
				PP_FILL, PBFS_NORMAL,
				&rFill[i], NULL);
			if (hr != S_OK) {
				logHRESULT(L"DrawThemeBackground() fill", hr);
				goto fail;
			}
		} else
			// TODO check errors
			FillRect(s->dc, &rFill[i], GetSysColorBrush(sysColor));

	hr = S_OK;
fail:
	// TODO check errors
	if (theme != NULL)
		CloseThemeData(theme);
	return hr;
}

static HRESULT drawButtonPart(HRESULT hr, struct drawState *s)
{
	uiTableValue *value;
	WCHAR *wstr;
	bool enabled;
	HTHEME theme;
	RECT r;
	TEXTMETRICW tm;

	if (hr != S_OK)
		return hr;
	if (s->p->buttonModelColumn == -1)
		return S_OK;

	value = uiprivTableModelCellValue(s->model, s->iItem, s->p->buttonModelColumn);
	wstr = toUTF16(uiTableValueString(value));
	uiFreeTableValue(value);
	enabled = uiprivTableModelCellEditable(s->model, s->iItem, s->p->buttonClickableModelColumn);

	theme = OpenThemeData(s->t->hwnd, L"button");

	if (GetTextMetricsW(s->dc, &tm) == 0) {
		logLastError(L"GetTextMetricsW()");
		hr = E_FAIL;
		goto fail;
	}
	r = s->m->subitemBounds;

	if (theme != NULL) {
		int state;

		state = PBS_NORMAL;
		if (!enabled)
			state = PBS_DISABLED;
		hr = DrawThemeBackground(theme, s->dc,
			BP_PUSHBUTTON, state,
			&r, NULL);
		if (hr != S_OK) {
			logHRESULT(L"DrawThemeBackground()", hr);
			goto fail;
		}
		// TODO DT_EDITCONTROL?
		// TODO DT_PATH_ELLIPSIS DT_WORD_ELLIPSIS instead of DT_END_ELLIPSIS? a middle-ellipsis option would be ideal here
		// TODO is there a theme property we can get instead of hardcoding these flags? if not, make these flags a macro
		hr = DrawThemeText(theme, s->dc,
			BP_PUSHBUTTON, state,
			wstr, -1,
			DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX, 0,
			&r);
		if (hr != S_OK) {
			logHRESULT(L"DrawThemeText()", hr);
			goto fail;
		}
	} else {
		UINT state;
		HBRUSH color, prevColor;
		int prevBkMode;

		// TODO check errors
		// TODO explain why we're not doing this in the themed case (it has to do with extra transparent pixels)
		InflateRect(&r, -1, -1);
		state = DFCS_BUTTONPUSH;
		if (!enabled)
			state |= DFCS_INACTIVE;
		if (DrawFrameControl(s->dc, &r, DFC_BUTTON, state) == 0) {
			logLastError(L"DrawFrameControl()");
			hr = E_FAIL;
			goto fail;
		}
		color = GetSysColorBrush(COLOR_BTNTEXT);
		// TODO check errors for these two
		prevColor = (HBRUSH) SelectObject(s->dc, color);
		prevBkMode = SetBkMode(s->dc, TRANSPARENT);
		// TODO DT_EDITCONTROL?
		// TODO DT_PATH_ELLIPSIS DT_WORD_ELLIPSIS instead of DT_END_ELLIPSIS? a middle-ellipsis option would be ideal here
		if (DrawTextW(s->dc, wstr, -1, &r, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX) == 0) {
			logLastError(L"DrawTextW()");
			hr = E_FAIL;
			goto fail;
		}
		// TODO check errors for these two
		SetBkMode(s->dc, prevBkMode);
		SelectObject(s->dc, prevColor);
	}

	hr = S_OK;
fail:
	// TODO check errors
	if (theme != NULL)
		CloseThemeData(theme);
	uiprivFree(wstr);
	return hr;
}

static HRESULT freeDrawState(struct drawState *s)
{
	HRESULT hr, hrret;

	hrret = S_OK;
	if (s->m != NULL) {
		uiprivFree(s->m);
		s->m = NULL;
	}
	if (s->freeTextBrush) {
		if (DeleteObject(s->textBrush) == 0) {
			logLastError(L"DeleteObject()");
			hrret = E_FAIL;
			// continue cleaning up anyway
		}
		s->freeTextBrush = FALSE;
	}
	if (s->freeBgBrush) {
		if (DeleteObject(s->bgBrush) == 0) {
			logLastError(L"DeleteObject()");
			hrret = E_FAIL;
			// continue cleaning up anyway
		}
		s->freeBgBrush = FALSE;
	}
	return hrret;
}

static COLORREF blend(COLORREF base, double r, double g, double b, double a)
{
	double br, bg, bb;

	br = ((double) GetRValue(base)) / 255.0;
	bg = ((double) GetGValue(base)) / 255.0;
	bb = ((double) GetBValue(base)) / 255.0;

	br = (r * a) + (br * (1.0 - a));
	bg = (g * a) + (bg * (1.0 - a));
	bb = (b * a) + (bb * (1.0 - a));
	return RGB((BYTE) (br * 255),
		(BYTE) (bg * 255),
		(BYTE) (bb * 255));
}

static HRESULT fillDrawState(struct drawState *s, uiTable *t, NMLVCUSTOMDRAW *nm, uiprivTableColumnParams *p)
{
	LRESULT state;
	HWND header;
	HRESULT hr;

	ZeroMemory(s, sizeof (struct drawState));
	s->t = t;
	s->model = t->model;
	s->p = p;

	s->dc = nm->nmcd.hdc;
	s->iItem = nm->nmcd.dwItemSpec;
	s->iSubItem = nm->iSubItem;

	hr = uiprivTableGetMetrics(t, s->iItem, s->iSubItem, &(s->m));
	if (hr != S_OK)
		goto fail;

	if (s->m->selected) {
		s->bgColor = GetSysColor(COLOR_HIGHLIGHT);
		s->bgBrush = GetSysColorBrush(COLOR_HIGHLIGHT);
		s->textColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
		s->textBrush = GetSysColorBrush(COLOR_HIGHLIGHTTEXT);
	} else {
		double r, g, b, a;

		s->bgColor = GetSysColor(COLOR_WINDOW);
		s->bgBrush = GetSysColorBrush(COLOR_WINDOW);
		if (uiprivTableModelColorIfProvided(s->model, s->iItem, t->backgroundColumn, &r, &g, &b, &a)) {
			s->bgColor = blend(s->bgColor, r, g, b, a);
			s->bgBrush = CreateSolidBrush(s->bgColor);
			if (s->bgBrush == NULL) {
				logLastError(L"CreateSolidBrush()");
				hr = E_FAIL;
				goto fail;
			}
			s->freeBgBrush = TRUE;
		}
		s->textColor = GetSysColor(COLOR_WINDOWTEXT);
		s->textBrush = GetSysColorBrush(COLOR_WINDOWTEXT);
		if (uiprivTableModelColorIfProvided(s->model, s->iItem, p->textParams.ColorModelColumn, &r, &g, &b, &a)) {
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

	return S_OK;
fail:
	// ignore the error; we need to return the one we got above
	freeDrawState(s);
	return hr;
}

static HRESULT updateAndDrawFocusRects(HRESULT hr, uiTable *t, HDC dc, int iItem, RECT *realTextBackground, RECT *focus, bool *first)
{
	LRESULT state;

	if (hr != S_OK)
		return hr;
	if (GetFocus() != t->hwnd)
		return S_OK;
	// uItemState CDIS_FOCUS doesn't quite work right because of bugs in the Windows list view that causes spurious redraws without the flag while we hover over the focused item
	// TODO only call this once
	state = SendMessageW(t->hwnd, LVM_GETITEMSTATE, (WPARAM) iItem, (LRESULT) (LVIS_FOCUSED));
	if ((state & LVIS_FOCUSED) == 0)
		return S_OK;

	if (realTextBackground != NULL)
		if (*first) {
			*focus = *realTextBackground;
			*first = false;
			return S_OK;
		} else if (focus->right == realTextBackground->left) {
			focus->right = realTextBackground->right;
			return S_OK;
		}
	if (DrawFocusRect(dc, focus) == 0) {
		logLastError(L"DrawFocusRect()");
		return E_FAIL;
	}
	if (realTextBackground != NULL)
		*focus = *realTextBackground;
	return S_OK;
}

// normally we would only draw stuff in subitem stages
// this broke when we tried drawing focus rects in postpaint; the drawing either kept getting wiped or overdrawn, mouse hovering had something to do with it, and none of the "solutions" to this or similar problems on the internet worked
// so now we do everything in the item prepaint stage
// TODO consider switching to full-on owner draw
// TODO only compute the background brushes once?
HRESULT uiprivTableHandleNM_CUSTOMDRAW(uiTable *t, NMLVCUSTOMDRAW *nm, LRESULT *lResult)
{
	struct drawState s;
	uiprivTableColumnParams *p;
	NMLVCUSTOMDRAW b;
	size_t i, n;
	RECT focus;
	bool focusFirst;
	HRESULT hr;

	switch (nm->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*lResult = CDRF_NOTIFYITEMDRAW;
		return S_OK;
	case CDDS_ITEMPREPAINT:
		break;
	default:
		*lResult = CDRF_DODEFAULT;
		return S_OK;
	}

	n = t->columns->size();
	b = *nm;
	focusFirst = true;
	for (i = 0; i < n; i++) {
		b.iSubItem = i;
		p = (*(t->columns))[i];
		hr = fillDrawState(&s, t, &b, p);
		if (hr != S_OK)
			return hr;
		hr = drawBackgrounds(S_OK, &s);
		hr = drawImagePart(hr, &s);
		hr = drawCheckboxPart(hr, &s);
		hr = drawTextPart(hr, &s);
		hr = drawProgressBarPart(hr, &s);
		hr = drawButtonPart(hr, &s);
		hr = updateAndDrawFocusRects(hr, s.t, s.dc, nm->nmcd.dwItemSpec, &(s.m->realTextBackground), &focus, &focusFirst);
		if (hr != S_OK)
			goto fail;
		hr = freeDrawState(&s);
		if (hr != S_OK)		// TODO really error out here?
			return hr;
	}
	// and draw the last focus rect
	hr = updateAndDrawFocusRects(hr, t, nm->nmcd.hdc, nm->nmcd.dwItemSpec, NULL, &focus, &focusFirst);
	if (hr != S_OK)
		return hr;
	*lResult = CDRF_SKIPDEFAULT;
	return S_OK;
fail:
	// ignore error here
	// TODO this is awkward cleanup placement for something that only really exists in a for loop
	freeDrawState(&s);
	return hr;
}

// TODO run again when the DPI or the theme changes
// TODO properly clean things up here
// TODO properly destroy the old lists here too
HRESULT uiprivUpdateImageListSize(uiTable *t)
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
		hr = CloseThemeData(theme);
		if (hr != S_OK) {
			logHRESULT(L"CloseThemeData()", hr);
			return hr;
		}
	}

	// TODO handle errors
	t->imagelist = ImageList_Create(cxList, cyList,
		ILC_COLOR32,
		1, 1);
	if (t->imagelist == NULL) {
		logLastError(L"ImageList_Create()");
		return E_FAIL;
	}
	// TODO will this return NULL here because it's an initial state?
	SendMessageW(t->hwnd, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM) (t->imagelist));

	if (ReleaseDC(t->hwnd, dc) == 0) {
		logLastError(L"ReleaseDC()");
		return E_FAIL;
	}
	return S_OK;
}
