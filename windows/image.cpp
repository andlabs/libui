#include "uipriv_windows.hpp"

IWICImagingFactory *uiprivWICFactory = NULL;

HRESULT uiprivInitImage(void)
{
	return CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory, (void **) (&uiprivWICFactory));
}

void uiprivUninitImage(void)
{
	uiprivWICFactory->Release();
	uiprivWICFactory = NULL;
}

struct uiImage {
	double width;
	double height;
	std::vector<IWICBitmap *> *bitmaps;
};

uiImage *uiNewImage(double width, double height)
{
	uiImage *i;

	i = uiprivNew(uiImage);
	i->width = width;
	i->height = height;
	i->bitmaps = new std::vector<IWICBitmap *>;
	return i;
}

void uiFreeImage(uiImage *i)
{
	for (IWICBitmap *b : *(i->bitmaps))
		b->Release();
	delete i->bitmaps;
	uiprivFree(i);
}

void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int pixelStride)
{
	IWICBitmap *b;
	HRESULT hr;

	hr = uiprivWICFactory->CreateBitmapFromMemory(pixelWidth, pixelHeight,
		GUID_WICPixelFormat32bppRGBA, pixelStride,
		pixelStride * pixelHeight, (BYTE *) pixels,
		&b);
	if (hr != S_OK)
		logHRESULT(L"error calling CreateBitmapFromMemory() in uiImageAppend()", hr);
	i->bitmaps->push_back(b);
}

struct matcher {
	IWICBitmap *best;
	int distX;
	int distY;
	int targetX;
	int targetY;
	bool foundLarger;
};

// TODO is this the right algorithm?
static void match(IWICBitmap *b, struct matcher *m)
{
	UINT ux, uy;
	int x, y;
	int x2, y2;
	HRESULT hr;

	hr = b->GetSize(&ux, &uy);
	if (hr != S_OK)
		logHRESULT(L"error calling GetSize() in match()", hr);
	x = ux;
	y = uy;
	if (m->best == NULL)
		goto writeMatch;

	if (x < m->targetX && y < m->targetY)
		if (m->foundLarger)
			// always prefer larger ones
			return;
	if (x >= m->targetX && y >= m->targetY && !m->foundLarger)
		// we set foundLarger below
		goto writeMatch;

// TODO
#define abs(x) ((x) < 0 ? -(x) : (x))
	x2 = abs(m->targetX - x);
	y2 = abs(m->targetY - y);
	if (x2 < m->distX && y2 < m->distY)
		goto writeMatch;

	// TODO weight one dimension? threshhold?
	return;

writeMatch:
	// must set this here too; otherwise the first image will never have ths set
	if (x >= m->targetX && y >= m->targetY && !m->foundLarger)
		m->foundLarger = true;
	m->best = b;
	m->distX = abs(m->targetX - x);
	m->distY = abs(m->targetY - y);
}

IWICBitmap *uiprivImageAppropriateForDC(uiImage *i, HDC dc)
{
	struct matcher m;

	m.best = NULL;
	m.distX = INT_MAX;
	m.distY = INT_MAX;
	m.targetX = i->width * GetDeviceCaps(dc, LOGPIXELSX);
	m.targetY = i->height * GetDeviceCaps(dc, LOGPIXELSY);
	m.foundLarger = false;
	for (IWICBitmap *b : *(i->bitmaps))
		match(b, &m);
	return m.best;
}

// TODO see if we can really pass NULL to CreateDIBSection()'s HDC parameter, and if so, use HBITMAPs before WIC maybe?
HBITMAP uiprivWICToGDI(IWICBitmap *b, HDC dc)
{
	BITMAPINFO bmi;
	UINT width, height;
	HBITMAP hb;
	VOID *bits;
	BITMAP bmp;
	HRESULT hr;

	ZeroMemory(&bmi, sizeof (BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	hr = b->GetSize(&width, &height);
	if (hr != S_OK)
		logHRESULT(L"error calling GetSize() in uiprivWICToGDI()", hr);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -((int) height);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	hb = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS,
		&bits, NULL, 0);
	if (hb == NULL)
		logLastError(L"error calling CreateDIBSection() in uiprivWICToGDI()");

	// now we need to figure out the stride of the image data GDI gave us
	// TODO find out if CreateDIBSection fills that in bmi for us
	if (GetObject(hb, sizeof (BITMAP), &bmp) == 0)
		logLastError(L"error calling GetObject() in uiprivWICToGDI()");
	hr = b->CopyPixels(NULL, bmp.bmWidthBytes,
		bmp.bmWidthBytes * bmp.bmHeight, (BYTE *) bits);
	if (hr != S_OK)
		logHRESULT(L"error calling CopyPixels() in uiprivWICToGDI()", hr);

	return hb;
}
