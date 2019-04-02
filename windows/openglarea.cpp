#include "uipriv_windows.hpp"
#include "area.hpp"
#include <GL/gl.h>

// WGL_ARB_extensions_string
typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);

// WGL_ARB_create_context
#define WGL_ARB_create_context
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define ERROR_INVALID_VERSION_ARB 0x2095
#define ERROR_INVALID_PROFILE_ARB 0x2096
typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);


// WGL_ARB_create_context_profile
#define WGL_ARB_create_context_profile
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// WGL_ARB_create_context_robustness
#define WGL_ARB_create_context_robustness
#define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB         0x00000004

// WGL_EXT_create_context_es2_profile
#define WGL_EXT_create_context_es2_profile
#define WGL_CONTEXT_ES2_PROFILE_BIT_EXT           0x00000004

// WGL_EXT_create_context_es_profile
#define WGL_EXT_create_context_es_profile
#define WGL_CONTEXT_ES_PROFILE_BIT_EXT            0x00000004

// WGL_ARB_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_DRAW_TO_BITMAP_ARB 0x2002
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_NEED_PALETTE_ARB 0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB 0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB 0x2006
#define WGL_SWAP_METHOD_ARB 0x2007
#define WGL_NUMBER_OVERLAYS_ARB 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB 0x2009
#define WGL_TRANSPARENT_ARB 0x200A
#define WGL_SHARE_DEPTH_ARB 0x200C
#define WGL_SHARE_STENCIL_ARB 0x200D
#define WGL_SHARE_ACCUM_ARB 0x200E
#define WGL_SUPPORT_GDI_ARB 0x200F
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_STEREO_ARB 0x2012
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_RED_BITS_ARB 0x2015
#define WGL_RED_SHIFT_ARB 0x2016
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_GREEN_SHIFT_ARB 0x2018
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_BLUE_SHIFT_ARB 0x201A
#define WGL_ALPHA_BITS_ARB 0x201B
#define WGL_ALPHA_SHIFT_ARB 0x201C
#define WGL_ACCUM_BITS_ARB 0x201D
#define WGL_ACCUM_RED_BITS_ARB 0x201E
#define WGL_ACCUM_GREEN_BITS_ARB 0x201F
#define WGL_ACCUM_BLUE_BITS_ARB 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_AUX_BUFFERS_ARB 0x2024
#define WGL_NO_ACCELERATION_ARB 0x2025
#define WGL_GENERIC_ACCELERATION_ARB 0x2026
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_SWAP_EXCHANGE_ARB 0x2028
#define WGL_SWAP_COPY_ARB 0x2029
#define WGL_SWAP_UNDEFINED_ARB 0x202A
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_TYPE_COLORINDEX_ARB 0x202C
#define WGL_TRANSPARENT_RED_VALUE_ARB 0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int* piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBFVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, FLOAT *pfValues);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, int *piValues);

// WGL_ARB_multisample
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042

// WGL_ARB_framebuffer_sRGB
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20A9


//Needs a context!
static BOOL WGLExtensionSupported(HDC hdc, const char *extension_name)
{
	//TODO check for WGL_ARB_extensions_string? how even?
	PFNWGLGETEXTENSIONSSTRINGARBPROC _wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC) wglGetProcAddress("wglGetExtensionsStringARB");

	if (strstr(_wglGetExtensionsStringARB(hdc), extension_name) == NULL)
		return false;

	return true;
}

static LRESULT CALLBACK areaWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiOpenGLArea *a;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	RECT client;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	LRESULT lResult;

	a = (uiOpenGLArea *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (a == NULL) {
		if (uMsg == WM_CREATE) {
			a = (uiOpenGLArea *) (cs->lpCreateParams);
			// assign a->hwnd here so we can use it immediately
			a->hwnd = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) a);

			uiOpenGLAttributes *attribs = a->attribs;
			PIXELFORMATDESCRIPTOR pfd = {
				sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
				1,                             // version number
				(DWORD) PFD_DRAW_TO_WINDOW |   // support window
					PFD_SUPPORT_OPENGL |       // support OpenGL
					(attribs->DoubleBuffer ? PFD_DOUBLEBUFFER : 0) | // double buffering
					(attribs->Stereo ? PFD_STEREO : 0),              // stereo
				PFD_TYPE_RGBA,	 // rgba, not color index
				(BYTE) (attribs->RedBits + attribs->GreenBits + attribs->BlueBits), //color depth (excl. alpha)
				0, 0, 0, 0, 0, 0,      // color bits ignored
				attribs->AlphaBits,    // no alpha buffer
				0,                     // shift bit ignored
				0,                     // no accumulation buffer
				0, 0, 0, 0,            // accum bits ignored
				attribs->DepthBits,    // depth buffer
				attribs->StencilBits,  // stencil buffer
				0,                     // no auxiliary buffer
				PFD_MAIN_PLANE,        // main layer
				0,		               // reserved
				0, 0, 0		           // layer masks ignored
			};

			a->hDC = GetDC(a->hwnd);
			if (a->hDC == NULL)
				logLastError(L"error getting DC for OpenGL context");

			int iPixelFormat = ChoosePixelFormat(a->hDC, &pfd);
			if (iPixelFormat == 0)
				logLastError(L"error getting pixel format for OpenGL context");

			if (SetPixelFormat(a->hDC, iPixelFormat, &pfd) == FALSE)
				logLastError(L"error setting OpenGL pixel format for device");

			HGLRC tempContext = wglCreateContext(a->hDC);
			if (tempContext == NULL)
				logLastError(L"error creating temporary OpenGL context");

			wglMakeCurrent(a->hDC, tempContext);

			/*
				// TODO
				// FIXME
				Unfortunately, Windows does not allow the user to change the pixel format of a
				window. You get to set it exactly once. Therefore, if you want to use a different pixel
				format from the one your fake context used (for sRGB or multisample framebuffers, or
				just different bit-depths of buffers), you must destroy the window entirely and recreate
				it after we are finished with the dummy context.

				An OpenGL window has its own pixel format. Because of this, only device contexts
				retrieved for the client area of an OpenGL window are allowed to draw into the window.
				As a result, an OpenGL window should be created with the WS_CLIPCHILDREN and
				WS_CLIPSIBLINGS styles. Additionally, the window class attribute should not
				include the CS_PARENTDC style.
			*/

			/*if(WGLExtensionSupported(a->hDC, "WGL_ARB_pixel_format")) {
				PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
				const int pixelAttribs[25] = {
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
					WGL_DOUBLE_BUFFER_ARB, attribs->DoubleBuffer ? GL_TRUE : GL_FALSE,
					WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, // TODO this excludes software renderer!?
					WGL_COLOR_BITS_ARB, attribs->RedBits + attribs->GreenBits + attribs->BlueBits,
					WGL_ALPHA_BITS_ARB, attribs->AlphaBits,
					WGL_DEPTH_BITS_ARB, attribs->DepthBits,
					WGL_STENCIL_BITS_ARB, attribs->StencilBits,
					0, //WGL_SAMPLE_BUFFERS_ARB, attribs->Samples > 0 ? GL_TRUE : GL_FALSE,
					0, //WGL_SAMPLES_ARB, attribs->Samples,
					0, //WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, attribs->SRGBCapable ? GL_TRUE : GL_FALSE,
					0
				};
				int nextIndex = 18;


				if(attribs->Samples > 0){
					if(WGLExtensionSupported(a->hDC, "WGL_ARB_multisample")) {
						pixelAttribs[nextIndex++] = WGL_SAMPLE_BUFFERS_ARB;
						pixelAttribs[nextIndex++] = GL_TRUE;
						pixelAttribs[nextIndex++] = WGL_SAMPLES_ARB;
						pixelAttribs[nextIndex++] = attribs->Samples;
					} else {
						// TODO handle error, multisampling requested but not available
					}
				}

				if(attribs->SRGBCapable) {
					if(WGLExtensionSupported(a->hDC, "WGL_ARB_framebuffer_sRGB")) {
						pixelAttribs[nextIndex++] = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB
						pixelAttribs[nextIndex++] = GL_TRUE;
					} else {
						// TODO handle error, SRGB framebuffer requested but not availble
					}
				}

				int pixelFormatID;
				UINT numFormats;
				BOOL status = wglChoosePixelFormatARB(a->hDC, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);
				 
				if (status == FALSE || numFormats == 0)
				    logLastError(L"error getting pixel format for OpenGL context (wglChoosePixelFormatARB)");

				PIXELFORMATDESCRIPTOR pfdNew;
				DescribePixelFormat(a->hDC, pixelFormatID, sizeof(pfdNew), &pfdNew);
				SetPixelFormat(a->hDC, pixelFormatID, &pfdNew);
			} else {
				// TODO handle error, esp. multisampling not available
				// can continue if no multisampling and SRGB framebuffer was requested?
			}*/

			if(WGLExtensionSupported(a->hDC, "WGL_ARB_create_context") &&
			   WGLExtensionSupported(a->hDC, "WGL_ARB_create_context_profile")) {

				PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB
					= (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

				if(attribs->UseOpenGLES && !(WGLExtensionSupported(a->hDC, "WGL_EXT_create_context_es_profile") && WGLExtensionSupported(a->hDC, "WGL_EXT_create_context_es2_profile"))) {
					// TODO handle error (OpenGL ES requested but not available)
				}

				if(attribs->Robustness && !WGLExtensionSupported(a->hDC, "WGL_ARB_create_context_robustness")) {
					// TODO error handling (robustness requested but not available)
				}

				int contextAttribsPos = 4;
				int contextAttribs[12] = {
					WGL_CONTEXT_MAJOR_VERSION_ARB,
						attribs->MajorVersion,
					WGL_CONTEXT_MINOR_VERSION_ARB,
						attribs->MinorVersion,
					WGL_CONTEXT_FLAGS_ARB,
						(attribs->DebugContext ? WGL_CONTEXT_DEBUG_BIT_ARB : 0) |
						(attribs->ForwardCompat ? WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0) |
						(attribs->Robustness ? WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB : 0),
					WGL_CONTEXT_PROFILE_MASK_ARB,
						(attribs->CompatProfile ?
							WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB :
							WGL_CONTEXT_CORE_PROFILE_BIT_ARB) |
						(attribs->UseOpenGLES ? WGL_CONTEXT_ES_PROFILE_BIT_EXT : 0),
					0
				};

				a->hglrc = wglCreateContextAttribsARB(a->hDC, 0, contextAttribs);
				if (a->hglrc == NULL)
					logLastError(L"error creating OpenGL context");
			} else {
				// TODO handle error - fallback to wglCreateContext & ignore attribs?
				//   should be consistent with other platforms
			}

			wglMakeCurrent(a->hDC, NULL);
			if (wglDeleteContext(tempContext) == FALSE)
				logLastError(L"error releasing temporary OpenGL context");

			uiOpenGLAreaMakeCurrent(a);

			if (WGLExtensionSupported(a->hDC, "WGL_EXT_swap_control")) {
				a->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
			} else {
				// TODO warn about no vsync
			}

			(*(a->ah->InitGL))(a->ah, a);
		}

		// fall through to DefWindowProcW() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	 if (uMsg == WM_DESTROY || uMsg == WM_NCDESTROY) {
		if (a->hglrc) {
			if(wglMakeCurrent(NULL, NULL) == FALSE)
				logLastError(L"error unsetting OpenGL context");
			if (ReleaseDC(a->hwnd, a->hDC) == 0)
				logLastError(L"error releasing DC for OpenGL context");
			a->hDC = NULL;
			if (wglDeleteContext(a->hglrc) == FALSE)
				logLastError(L"error releasing OpenGL context");
			a->hglrc = NULL;
		}
	}

	// always recreate the render target if necessary
	if (a->rt == NULL)
		a->rt = makeHWNDRenderTarget(a->hwnd);

	if (areaDoDraw((uiArea *) a, uMsg, wParam, lParam, &lResult, TRUE) != FALSE)
		return lResult;

	if (uMsg == WM_WINDOWPOSCHANGED) {
		if ((wp->flags & SWP_NOSIZE) != 0)
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		uiWindowsEnsureGetClientRect(a->hwnd, &client);
		areaDrawOnResize((uiArea *) a, &client);
		areaScrollOnResize((uiArea *) a, &client);
		return 0;
	}

	if (areaDoScroll((uiArea *) a, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	if (areaDoEvents((uiArea *) a, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;

	// nothing done
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// control implementation

uiWindowsControlAllDefaults(uiOpenGLArea)

static void uiOpenGLAreaMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	// TODO
	*width = 0;
	*height = 0;
}

ATOM registerOpenGLAreaClass(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = openGLAreaClass;
	wc.lpfnWndProc = areaWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	// this is just to be safe; see the InvalidateRect() call in the WM_WINDOWPOSCHANGED handler for more details
	wc.style = CS_HREDRAW | CS_VREDRAW;
	return RegisterClassW(&wc);
}

void unregisterOpenGLArea(void)
{
	if (UnregisterClassW(openGLAreaClass, hInstance) == 0)
		logLastError(L"error unregistering uiOpenGLArea window class");
}

void uiOpenGLAreaSetSize(uiOpenGLArea *a, int width, int height)
{
	a->scrollWidth = width;
	a->scrollHeight = height;
	areaUpdateScroll((uiArea *) a);
}

void uiOpenGLAreaSetVSync(uiOpenGLArea *a, int v)
{
	uiOpenGLAreaMakeCurrent(a);
	if(a->wglSwapIntervalEXT != NULL)
		a->wglSwapIntervalEXT(v);
}

void uiOpenGLAreaQueueRedrawAll(uiOpenGLArea *a)
{
	// don't erase the background; we do that ourselves in doPaint()
	invalidateRect(a->hwnd, NULL, FALSE);
}

void uiOpenGLAreaMakeCurrent(uiOpenGLArea *a)
{
	if(wglMakeCurrent(a->hDC, a->hglrc) == FALSE)
		logLastError(L"error setting current OpenGL context");
}

void uiOpenGLAreaSwapBuffers(uiOpenGLArea *a)
{
	if(SwapBuffers(a->hDC) == FALSE)
		logLastError(L"error swapping OpenGL buffers");
}

void uiOpenGLAreaBeginUserWindowMove(uiOpenGLArea *a)
{
	HWND toplevel;

	// TODO restrict execution
	ReleaseCapture();		// TODO use properly and reset internal data structures
	toplevel = parentToplevel(a->hwnd);
	if (toplevel == NULL) {
		// TODO
		return;
	}
	// see http://stackoverflow.com/questions/40249940/how-do-i-initiate-a-user-mouse-driven-move-or-resize-for-custom-window-borders-o#40250654
	SendMessageW(toplevel, WM_SYSCOMMAND,
		SC_MOVE | 2, 0);
}

void uiOpenGLAreaBeginUserWindowResize(uiOpenGLArea *a, uiWindowResizeEdge edge)
{
	HWND toplevel;
	WPARAM wParam;

	// TODO restrict execution
	ReleaseCapture();		// TODO use properly and reset internal data structures
	toplevel = parentToplevel(a->hwnd);
	if (toplevel == NULL) {
		// TODO
		return;
	}
	// see http://stackoverflow.com/questions/40249940/how-do-i-initiate-a-user-mouse-driven-move-or-resize-for-custom-window-borders-o#40250654
	wParam = SC_SIZE;
	switch (edge) {
	case uiWindowResizeEdgeLeft:
		wParam |= 1;
		break;
	case uiWindowResizeEdgeTop:
		wParam |= 3;
		break;
	case uiWindowResizeEdgeRight:
		wParam |= 2;
		break;
	case uiWindowResizeEdgeBottom:
		wParam |= 6;
		break;
	case uiWindowResizeEdgeTopLeft:
		wParam |= 4;
		break;
	case uiWindowResizeEdgeTopRight:
		wParam |= 5;
		break;
	case uiWindowResizeEdgeBottomLeft:
		wParam |= 7;
		break;
	case uiWindowResizeEdgeBottomRight:
		wParam |= 8;
		break;
	}
	SendMessageW(toplevel, WM_SYSCOMMAND,
		wParam, 0);
}

uiOpenGLArea *uiNewOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs)
{
	uiOpenGLArea *a;

	uiWindowsNewControl(uiOpenGLArea, a);

	a->ah = ah;
	a->scrolling = FALSE;
	a->attribs = attribs;
	uiprivClickCounterReset(&(a->cc));

	// a->hwnd is assigned in areaWndProc()
	uiWindowsEnsureCreateControlHWND(0,
		openGLAreaClass, L"",
		0,
		hInstance, a,
		FALSE);

	return a;
}

// uiOpenGLArea *uiNewScrollingOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs, int width, int height)
// {
// 	uiOpenGLArea *a;

// 	uiWindowsNewControl(uiOpenGLArea, a);

// 	a->ah = ah;
// 	a->scrolling = TRUE;
// 	a->scrollWidth = width;
// 	a->scrollHeight = height;
// 	uiprivClickCounterReset(&(a->cc));

// 	// a->hwnd is assigned in areaWndProc()
// 	uiWindowsEnsureCreateControlHWND(0,
// 		areaClass, L"",
// 		WS_HSCROLL | WS_VSCROLL,
// 		hInstance, a,
// 		FALSE);

// 	// set initial scrolling parameters
// 	areaUpdateScroll((uiArea *) a);

// 	return a;
// }
