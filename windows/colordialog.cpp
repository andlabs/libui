// 16 may 2016
#include "uipriv_windows.hpp"

struct colorDialog {
	HWND hwnd;

	HWND svChooser;
	HWND hSlider;
	HWND preview;
	HWND opacitySlider;
	HWND editH;
	HWND editS;
	HWND editV;
	HWND editRDouble, editRInt;
	HWND editGDouble, editGInt;
	HWND editBDouble, editBInt;
	HWND editADouble, editAInt;
	HWND editHex;

	double r;
	double g;
	double b;
	double a;
	struct colorDialogRGBA *out;

	BOOL updating;
};

// both of these are from the wikipedia page on HSV
static void rgb2HSV(double r, double g, double b, double *h, double *s, double *v)
{
	double M, m;
	int whichmax;
	double c;

	M = r;
	whichmax = 0;
	if (M < g) {
		M = g;
		whichmax = 1;
	}
	if (M < b) {
		M = b;
		whichmax = 2;
	}
	m = r;
	if (m > g)
		m = g;
	if (m > b)
		m = b;
	c = M - m;

	if (c == 0)
		*h = 0;
	else {
		switch (whichmax) {
		case 0:
			*h = ((g - b) / c);
			if (*h > 6)
				*h -= 6;
			break;
		case 1:
			*h = ((b - r) / c) + 2;
			break;
		case 2:
			*h = ((r - g) / c) + 4;
			break;
		}
		*h /= 6;		// put in range [0,1)
	}

	*v = M;

	if (c == 0)
		*s = 0;
	else
		*s = c / *v;
}

static void hsv2RGB(double h, double s, double v, double *r, double *g, double *b)
{
	double c;
	double hPrime;
	int h60;
	double x;
	double m;
	double c1, c2;

	c = v * s;
	hPrime = h * 6;
	h60 = (int) hPrime;		// equivalent to splitting into 60° chunks
	x = c * (1.0 - fabs(fmod(hPrime, 2) - 1.0));
	m = v - c;
	switch (h60) {
	case 0:
		*r = c + m;
		*g = x + m;
		*b = m;
		return;
	case 1:
		*r = x + m;
		*g = c + m;
		*b = m;
		return;
	case 2:
		*r = m;
		*g = c + m;
		*b = x + m;
		return;
	case 3:
		*r = m;
		*g = x + m;
		*b = c + m;
		return;
	case 4:
		*r = x + m;
		*g = m;
		*b = c + m;
		return;
	case 5:
		*r = c + m;
		*g = m;
		*b = x + m;
		return;
	}
	// TODO
}

static void updateDouble(HWND hwnd, double d, HWND whichChanged)
{
	WCHAR *str;

	if (whichChanged == hwnd)
		return;
	str = ftoutf16(d);
	setWindowText(hwnd, str);
	uiFree(str);
}

static void updateDialog(struct colorDialog *c, HWND whichChanged)
{
	double h, s, v;
	uint8_t rb, gb, bb, ab;
	WCHAR *str;

	c->updating = TRUE;

	rgb2HSV(c->r, c->g, c->b, &h, &s, &v);

	updateDouble(c->editH, h, whichChanged);
	updateDouble(c->editS, s, whichChanged);
	updateDouble(c->editV, v, whichChanged);

	updateDouble(c->editRDouble, c->r, whichChanged);
	updateDouble(c->editGDouble, c->g, whichChanged);
	updateDouble(c->editBDouble, c->b, whichChanged);
	updateDouble(c->editADouble, c->a, whichChanged);

	rb = (uint8_t) (c->r * 255);
	gb = (uint8_t) (c->g * 255);
	bb = (uint8_t) (c->b * 255);
	ab = (uint8_t) (c->a * 255);

	if (whichChanged != c->editRInt) {
		str = itoutf16(rb);
		setWindowText(c->editRInt, str);
		uiFree(str);
	}
	if (whichChanged != c->editGInt) {
		str = itoutf16(gb);
		setWindowText(c->editGInt, str);
		uiFree(str);
	}
	if (whichChanged != c->editBInt) {
		str = itoutf16(bb);
		setWindowText(c->editBInt, str);
		uiFree(str);
	}
	if (whichChanged != c->editAInt) {
		str = itoutf16(ab);
		setWindowText(c->editAInt, str);
		uiFree(str);
	}

	if (whichChanged != c->editHex) {
		// TODO hex
	}

	// TODO TRUE?
	invalidateRect(c->svChooser, NULL, TRUE);
//TODO	invalidateRect(c->hSlider, NULL, TRUE);
//TODO	invalidateRect(c->preview, NULL, TRUE);
//TODO	invalidateRect(c->opacitySlider, NULL, TRUE);

	c->updating = FALSE;
}

// this interesting approach comes from http://blogs.msdn.com/b/wpfsdk/archive/2006/10/26/uncommon-dialogs--font-chooser-and-color-picker-dialogs.aspx
static void drawSVChooser(struct colorDialog *c, ID2D1RenderTarget *rt)
{
	D2D1_SIZE_F size;
	D2D1_RECT_F rect;
	double h, s, v;
	double rTop, gTop, bTop;
	D2D1_GRADIENT_STOP stops[2];
	ID2D1GradientStopCollection *collection;
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lprop;
	D2D1_BRUSH_PROPERTIES bprop;
	ID2D1LinearGradientBrush *brush;
	ID2D1LinearGradientBrush *opacity;
	ID2D1Layer *layer;
	D2D1_LAYER_PARAMETERS layerparams;
	D2D1_ELLIPSE mparam;
	D2D1_COLOR_F mcolor;
	ID2D1SolidColorBrush *markerBrush;
	HRESULT hr;

	size = rt->GetSize();
	rect.left = 0;
	rect.top = 0;
	rect.right = size.width;
	rect.bottom = size.height;

	// TODO draw checkerboard

	// first, draw a vertical gradient from the current hue at max S/V to black
	// the source example draws it upside down; let's do so too just to be safe
	rgb2HSV(c->r, c->g, c->b, &h, &s, &v);
	hsv2RGB(h, 1.0, 1.0, &rTop, &gTop, &bTop);
	stops[0].position = 0;
	stops[0].color.r = 0.0;
	stops[0].color.g = 0.0;
	stops[0].color.b = 0.0;
	stops[0].color.a = 1.0;
	stops[1].position = 1;
	stops[1].color.r = rTop;
	stops[1].color.g = gTop;
	stops[1].color.b = bTop;
	stops[1].color.a = 1.0;
	hr = rt->CreateGradientStopCollection(stops, 2,
		D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP,
		&collection);
	if (hr != S_OK)
		logHRESULT(L"error making gradient stop collection for first gradient in SV chooser", hr);
	ZeroMemory(&lprop, sizeof (D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES));
	lprop.startPoint.x = size.width / 2;
	lprop.startPoint.y = size.height;
	lprop.endPoint.x = size.width / 2;
	lprop.endPoint.y = 0;
	ZeroMemory(&bprop, sizeof (D2D1_BRUSH_PROPERTIES));
	bprop.opacity = 1.0;
	bprop.transform._11 = 1;
	bprop.transform._22 = 1;
	hr = rt->CreateLinearGradientBrush(&lprop, &bprop,
		collection, &brush);
	if (hr != S_OK)
		logHRESULT(L"error making gradient brush for first gradient in SV chooser", hr);
	rt->FillRectangle(&rect, brush);
	brush->Release();
	collection->Release();

	// second, create an opacity mask for the third step: a horizontal gradientthat goes from opaque to translucent
	stops[0].position = 0;
	stops[0].color.r = 0.0;
	stops[0].color.g = 0.0;
	stops[0].color.b = 0.0;
	stops[0].color.a = 1.0;
	stops[1].position = 1;
	stops[1].color.r = 0.0;
	stops[1].color.g = 0.0;
	stops[1].color.b = 0.0;
	stops[1].color.a = 0.0;
	hr = rt->CreateGradientStopCollection(stops, 2,
		D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP,
		&collection);
	if (hr != S_OK)
		logHRESULT(L"error making gradient stop collection for opacity mask gradient in SV chooser", hr);
	ZeroMemory(&lprop, sizeof (D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES));
	lprop.startPoint.x = 0;
	lprop.startPoint.y = size.height / 2;
	lprop.endPoint.x = size.width;
	lprop.endPoint.y = size.height / 2;
	ZeroMemory(&bprop, sizeof (D2D1_BRUSH_PROPERTIES));
	bprop.opacity = 1.0;
	bprop.transform._11 = 1;
	bprop.transform._22 = 1;
	hr = rt->CreateLinearGradientBrush(&lprop, &bprop,
		collection, &opacity);
	if (hr != S_OK)
		logHRESULT(L"error making gradient brush for opacity mask gradient in SV chooser", hr);
	collection->Release();

	// finally, make a vertical gradient from white at the top to black at the bottom (right side up this time) and with the previous opacity mask
	stops[0].position = 0;
	stops[0].color.r = 1.0;
	stops[0].color.g = 1.0;
	stops[0].color.b = 1.0;
	stops[0].color.a = 1.0;
	stops[1].position = 1;
	stops[1].color.r = 0.0;
	stops[1].color.g = 0.0;
	stops[1].color.b = 0.0;
	stops[1].color.a = 1.0;
	hr = rt->CreateGradientStopCollection(stops, 2,
		D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP,
		&collection);
	if (hr != S_OK)
		logHRESULT(L"error making gradient stop collection for second gradient in SV chooser", hr);
	ZeroMemory(&lprop, sizeof (D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES));
	lprop.startPoint.x = size.width / 2;
	lprop.startPoint.y = 0;
	lprop.endPoint.x = size.width / 2;
	lprop.endPoint.y = size.height;
	ZeroMemory(&bprop, sizeof (D2D1_BRUSH_PROPERTIES));
	bprop.opacity = 1.0;
	bprop.transform._11 = 1;
	bprop.transform._22 = 1;
	hr = rt->CreateLinearGradientBrush(&lprop, &bprop,
		collection, &brush);
	if (hr != S_OK)
		logHRESULT(L"error making gradient brush for second gradient in SV chooser", hr);
	// oh but wait we can't use FillRectangle() with an opacity mask
	// and we can't use FillGeometry() with both an opacity mask and a non-bitmap
	// layers it is!
	hr = rt->CreateLayer(&size, &layer);
	if (hr != S_OK)
		logHRESULT(L"error making layer for second gradient in SV chooser", hr);
	ZeroMemory(&layerparams, sizeof (D2D1_LAYER_PARAMETERS));
	layerparams.contentBounds = rect;
	// TODO make sure these are right
	layerparams.geometricMask = NULL;
	layerparams.maskAntialiasMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
	layerparams.maskTransform._11 = 1;
	layerparams.maskTransform._22 = 1;
	layerparams.opacity = 1.0;
	layerparams.opacityBrush = opacity;
	layerparams.layerOptions = D2D1_LAYER_OPTIONS_NONE;
	rt->PushLayer(&layerparams, layer);
	rt->FillRectangle(&rect, brush);
	rt->PopLayer();
	layer->Release();
	brush->Release();
	collection->Release();
	opacity->Release();

	// and now we just draw the marker
	ZeroMemory(&mparam, sizeof (D2D1_ELLIPSE));
	mparam.point.x = s * size.width;
	mparam.point.y = (1 - v) * size.height;
	mparam.radiusX = 7;
	mparam.radiusY = 7;
	// TODO make the color contrast?
	mcolor.r = 1.0;
	mcolor.g = 1.0;
	mcolor.b = 1.0;
	mcolor.a = 1.0;
	hr = rt->CreateSolidColorBrush(&mcolor, &bprop, &markerBrush);
	if (hr != S_OK)
		logHRESULT(L"error creating brush for SV chooser", hr);
	rt->DrawEllipse(&mparam, markerBrush, 2, NULL);
	markerBrush->Release();
}

static LRESULT CALLBACK svChooserSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	ID2D1RenderTarget *rt;
	struct colorDialog *c;

	switch (uMsg) {
	case msgD2DScratchPaint:
		rt = (ID2D1RenderTarget *) lParam;
		c = (struct colorDialog *) dwRefData;
		drawSVChooser(c, rt);
		return 0;
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, svChooserSubProc, uIdSubclass) == FALSE)
			logLastError(L"error removing color dialog SV chooser subclass");
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

// TODO extract into d2dscratch.cpp, use in font dialog
HWND replaceWithD2DScratch(HWND parent, int id, SUBCLASSPROC subproc, void *data)
{
	HWND replace;
	RECT r;

	replace = getDlgItem(parent, id);
	uiWindowsEnsureGetWindowRect(replace, &r);
	mapWindowRect(NULL, parent, &r);
	uiWindowsEnsureDestroyWindow(replace);
	return newD2DScratch(parent, &r, (HMENU) id, subproc, (DWORD_PTR) data);
	// TODO preserve Z-order
}

// a few issues:
// - some controls are positioned wrong; see http://stackoverflow.com/questions/37263267/why-are-some-of-my-controls-positioned-slightly-off-in-a-dialog-template-in-a-re
// - labels are too low; need to adjust them by the font's internal leading
// fixupControlPositions() and the following helper routines fix that for us

static LONG offsetTo(HWND a, HWND b)
{
	RECT ra, rb;

	uiWindowsEnsureGetWindowRect(a, &ra);
	uiWindowsEnsureGetWindowRect(b, &rb);
	return rb.top - ra.bottom;
}

static void moveWindowsUp(struct colorDialog *c, LONG by, ...)
{
	va_list ap;
	HWND cur;
	RECT r;

	va_start(ap, by);
	for (;;) {
		cur = va_arg(ap, HWND);
		if (cur == NULL)
			break;
		uiWindowsEnsureGetWindowRect(cur, &r);
		mapWindowRect(NULL, c->hwnd, &r);
		r.top -= by;
		r.bottom -= by;
		// TODO this isn't technically during a resize
		uiWindowsEnsureMoveWindowDuringResize(cur,
			r.left, r.top,
			r.right - r.left, r.bottom - r.top);
	}
	va_end(ap);
}

static void fixupControlPositions(struct colorDialog *c)
{
	HWND labelH;
	HWND labelS;
	HWND labelV;
	HWND labelR;
	HWND labelG;
	HWND labelB;
	HWND labelA;
	HWND labelHex;
	LONG offset;
	uiWindowsSizing sizing;

	labelH = getDlgItem(c->hwnd, rcHLabel);
	labelS = getDlgItem(c->hwnd, rcSLabel);
	labelV = getDlgItem(c->hwnd, rcVLabel);
	labelR = getDlgItem(c->hwnd, rcRLabel);
	labelG = getDlgItem(c->hwnd, rcGLabel);
	labelB = getDlgItem(c->hwnd, rcBLabel);
	labelA = getDlgItem(c->hwnd, rcALabel);
	labelHex = getDlgItem(c->hwnd, rcHexLabel);

	offset = offsetTo(c->editH, c->editS);
	moveWindowsUp(c, offset,
		labelS, c->editS,
		labelG, c->editGDouble, c->editGInt,
		NULL);
	offset = offsetTo(c->editS, c->editV);
	moveWindowsUp(c, offset,
		labelV, c->editV,
		labelB, c->editBDouble, c->editBInt,
		NULL);
	offset = offsetTo(c->editBDouble, c->editADouble);
	moveWindowsUp(c, offset,
		labelA, c->editADouble, c->editAInt,
		NULL);

	// TODO this uses the message font, not the dialog font
	uiWindowsGetSizing(c->hwnd, &sizing);
	offset = sizing.InternalLeading;
	moveWindowsUp(c, offset,
		labelH, labelS, labelV,
		labelR, labelG, labelB, labelA,
		labelHex,
		NULL);
}

static struct colorDialog *beginColorDialog(HWND hwnd, LPARAM lParam)
{
	struct colorDialog *c;

	c = uiNew(struct colorDialog);
	c->hwnd = hwnd;
	c->out = (struct colorDialogRGBA *) lParam;
	c->r = c->out->r;		// load initial values now
	c->g = c->out->g;
	c->b = c->out->b;
	c->a = c->out->a;

	// TODO set up d2dscratches

	// TODO prefix all these with rcColor instead of just rc
	c->editH = getDlgItem(c->hwnd, rcH);
	c->editS = getDlgItem(c->hwnd, rcS);
	c->editV = getDlgItem(c->hwnd, rcV);
	c->editRDouble = getDlgItem(c->hwnd, rcRDouble);
	c->editRInt = getDlgItem(c->hwnd, rcRInt);
	c->editGDouble = getDlgItem(c->hwnd, rcGDouble);
	c->editGInt = getDlgItem(c->hwnd, rcGInt);
	c->editBDouble = getDlgItem(c->hwnd, rcBDouble);
	c->editBInt = getDlgItem(c->hwnd, rcBInt);
	c->editADouble = getDlgItem(c->hwnd, rcADouble);
	c->editAInt = getDlgItem(c->hwnd, rcAInt);
	c->editHex = getDlgItem(c->hwnd, rcHex);

	c->svChooser = replaceWithD2DScratch(c->hwnd, rcColorSVChooser, svChooserSubProc, c);

	fixupControlPositions(c);

	// and get the ball rolling
	updateDialog(c, NULL);
	return c;
}

static double editDouble(HWND hwnd)
{
	WCHAR *s;
	double d;

	s = windowText(hwnd);
	d = _wtof(s);
	uiFree(s);
	return d;
}

static void hChanged(struct colorDialog *c)
{
	double h, s, v;

	rgb2HSV(c->r, c->g, c->b, &h, &s, &v);
	h = editDouble(c->editH);
	if (h < 0 || h >= 1.0)		// note the >=
		return;
printf("%g %g %g | ", c->r, c->g, c->b);
	hsv2RGB(h, s, v, &(c->r), &(c->g), &(c->b));
printf("%g %g %g\n", c->r, c->g, c->b);
	updateDialog(c, c->editH);
}

static void sChanged(struct colorDialog *c)
{
	double h, s, v;

	rgb2HSV(c->r, c->g, c->b, &h, &s, &v);
	s = editDouble(c->editS);
	if (s < 0 || s > 1)
		return;
	hsv2RGB(h, s, v, &(c->r), &(c->g), &(c->b));
	updateDialog(c, c->editS);
}

static void vChanged(struct colorDialog *c)
{
	double h, s, v;

	rgb2HSV(c->r, c->g, c->b, &h, &s, &v);
	v = editDouble(c->editV);
	if (v < 0 || v > 1)
		return;
	hsv2RGB(h, s, v, &(c->r), &(c->g), &(c->b));
	updateDialog(c, c->editV);
}

static void endColorDialog(struct colorDialog *c, INT_PTR code)
{
	if (EndDialog(c->hwnd, code) == 0)
		logLastError(L"error ending color dialog");
	uiFree(c);
}

// TODO make this void on the font dialog too
static void tryFinishDialog(struct colorDialog *c, WPARAM wParam)
{
	// cancelling
	if (LOWORD(wParam) != IDOK) {
		endColorDialog(c, 1);
		return;
	}

	// OK
	c->out->r = c->r;
	c->out->g = c->g;
	c->out->b = c->b;
	c->out->a = c->a;
	endColorDialog(c, 2);
}

// TODO change fontdialog to use this
// note that if we make this const, we get lots of weird compiler errors
static std::map<int, void (*)(struct colorDialog *)> changed = {
	{ rcH, hChanged },
	{ rcS, sChanged },
	{ rcV, vChanged },
};

static INT_PTR CALLBACK colorDialogDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct colorDialog *c;

	c = (struct colorDialog *) GetWindowLongPtrW(hwnd, DWLP_USER);
	if (c == NULL) {
		if (uMsg == WM_INITDIALOG) {
			c = beginColorDialog(hwnd, lParam);
			SetWindowLongPtrW(hwnd, DWLP_USER, (LONG_PTR) c);
			return TRUE;
		}
		return FALSE;
	}

	switch (uMsg) {
	case WM_COMMAND:
		SetWindowLongPtrW(c->hwnd, DWLP_MSGRESULT, 0);		// just in case
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			if (HIWORD(wParam) != BN_CLICKED)
				return FALSE;
			tryFinishDialog(c, wParam);
			return TRUE;
		case rcH:
		case rcS:
		case rcV:
			if (HIWORD(wParam) != EN_CHANGE)
				return FALSE;
			if (c->updating)		// prevent infinite recursion during an update
				return FALSE;
			(*(changed[LOWORD(wParam)]))(c);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL showColorDialog(HWND parent, struct colorDialogRGBA *c)
{
	switch (DialogBoxParamW(hInstance, MAKEINTRESOURCE(rcColorDialog), parent, colorDialogDlgProc, (LPARAM) c)) {
	case 1:			// cancel
		return FALSE;
	case 2:			// ok
		// make the compiler happy by putting the return after the switch
		break;
	default:
		logLastError(L"error running color dialog");
	}
	return TRUE;
}
