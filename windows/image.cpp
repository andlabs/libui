// 13 september 2016
#include "uipriv_windows.hpp"

struct uiImage {
	uiWindowsControl c;
	HWND hwnd;
	WCHAR *wfilename;
	HBITMAP bmpSource;
	HDC hdcSource;
	LONG width;
	LONG height;
};

static HBITMAP ConvertToHBitmap(IWICBitmapSource * ipBitmap)
{
	HBITMAP hbmp = NULL;
	void * pvImageBits = NULL;
	HDC hdcScreen = GetDC(NULL);

	UINT width = 0;
	UINT height = 0;
	if (FAILED(ipBitmap->GetSize(&width, &height)) || width == 0 || height == 0) {
		return NULL;
	}

	BITMAPINFO bminfo = {0};
	bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bminfo.bmiHeader.biWidth = width;
	bminfo.bmiHeader.biHeight = -((LONG) height);
	bminfo.bmiHeader.biPlanes = 1;
	bminfo.bmiHeader.biBitCount = 32;
	bminfo.bmiHeader.biCompression = BI_RGB;

	hbmp = CreateDIBSection(hdcScreen, &bminfo, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
	ReleaseDC(NULL, hdcScreen);
	if (hbmp == NULL) {
		return NULL;
	}

	UINT stride = width * 4;
	if (FAILED(ipBitmap->CopyPixels(NULL, stride, stride * height, static_cast<BYTE *>(pvImageBits)))) {
		DeleteObject(hbmp);
		hbmp = NULL;
	}

	return hbmp;
}

static HBITMAP LoadImageFromFile(const WCHAR *filename)
{
	IWICImagingFactory *wicFactory = NULL;
	IWICBitmapDecoder *decoder = NULL;
	IWICBitmapFrameDecode *frame = NULL;
	IWICBitmapSource *bitmap = NULL;
	HRESULT hr;

	hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&wicFactory)
	);

	if (FAILED(hr)) {
		return NULL;
	}

	hr = wicFactory->CreateDecoderFromFilename(
		filename,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&decoder
	);

	if (SUCCEEDED(hr)) {
		hr = decoder->GetFrame(0, &frame);
	}

	WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, frame, &bitmap);
	frame->Release();
	decoder->Release();

	return ConvertToHBitmap(bitmap);
}

static LRESULT CALLBACK imageWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiImage *i;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	PAINTSTRUCT ps;
	HDC hdcDestination;
	BITMAP bm = {0};

	i = (uiImage *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (i == NULL) {
		if (uMsg == WM_CREATE) {
			i = (uiImage *) (cs->lpCreateParams);
			// assign i->hwnd here so we can use it immediately
			i->hwnd = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) i);

			// Load the image through the windows imaging component
			i->bmpSource = LoadImageFromFile(i->wfilename);

			if(i->bmpSource == NULL) {
				logLastError(L"Failed to load the image");
			}
			GetObject(i->bmpSource, sizeof(bm), &bm);
			i->width = bm.bmWidth;
			i->height = bm.bmHeight;
			i->hdcSource = CreateCompatibleDC(GetDC(0));
			SelectObject(i->hdcSource, i->bmpSource);
			return 0;
		}
		// fall through to DefWindowProcW() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	if (uMsg == WM_PAINT) {
		hdcDestination = BeginPaint(hwnd, &ps);
		BitBlt(hdcDestination, 0, 0, i->width, i->height, i->hdcSource, 0, 0, SRCCOPY);
		EndPaint(hwnd, &ps);
		return 0;
	}

	// nothing done
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void uiImageDestroy(uiControl *c)
{
	uiImage *i = uiImage(c);

	uiFree(i->wfilename);
	uiWindowsEnsureDestroyWindow(i->hwnd);
	uiFreeControl(uiControl(i));
}

uiWindowsControlAllDefaultsExceptDestroy(uiImage)

static void uiImageMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiImage *i = uiImage(c);
	*width = i->width;
	*height = i->height;
}

void unregisterImage(void)
{
	if (UnregisterClassW(imageClass, hInstance) == 0)
		logLastError(L"error unregistering uiImage window class");
}

ATOM registerImageClass(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = imageClass;
	wc.lpfnWndProc = imageWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	return RegisterClassW(&wc);
}

uiImage *uiNewImage(const char *filename)
{
	uiImage *image;

	uiWindowsNewControl(uiImage, image);
	image->wfilename = toUTF16(filename);

	uiWindowsEnsureCreateControlHWND(0,
		imageClass, L"",
		0,
		hInstance, image,
		FALSE);

	return image;
}
