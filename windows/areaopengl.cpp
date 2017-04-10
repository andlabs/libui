#include "uipriv_windows.hpp"
#include "area.hpp"
#include <GL/gl.h>

void uiAreaOpenGLInit(uiArea * a) {
	PIXELFORMATDESCRIPTOR pfd = {
    	sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
    	1,		     // version number
    	PFD_DRAW_TO_WINDOW |   // support window
    		PFD_SUPPORT_OPENGL |   // support OpenGL
    		PFD_DOUBLEBUFFER,      // double buffered
    	PFD_TYPE_RGBA,	 // RGBA type
    	24,		    // 24-bit color depth
    	0, 0, 0, 0, 0, 0,      // color bits ignored
    	0,		     // no alpha buffer
    	0,		     // shift bit ignored
    	0,		     // no accumulation buffer
    	0, 0, 0, 0,	    // accum bits ignored
    	32,		    // 32-bit z-buffer
    	0,		     // no stencil buffer
    	0,		     // no auxiliary buffer
    	PFD_MAIN_PLANE,	// main layer
    	0,		     // reserved
    	0, 0, 0		// layer masks ignored
	};

	int  iPixelFormat;

	a->hDC = GetDC(a->hwnd);

	// get the best available match of pixel format for the device context
	iPixelFormat = ChoosePixelFormat(a->hDC, &pfd);

	// make that the pixel format of the device context
	SetPixelFormat(a->hDC, iPixelFormat, &pfd);

    a->hglrc = wglCreateContext(a->hDC);
    wglMakeCurrent(a->hDC, a->hglrc);
}

void uiAreaOpenGLBeginDraw(uiArea * a) {
	RECT rect;
	GetClientRect(a->hwnd,&rect);
	glViewport(0, 0, rect.right, rect.bottom);
    
	COLORREF bgcolorref = GetSysColor(COLOR_BTNFACE);
    float r = ((float) GetRValue(bgcolorref)) / 255.0;
	// due to utter apathy on Microsoft's part, GetGValue() does not work with MSVC's Run-Time Error Checks
	// it has not worked since 2008 and they have *never* fixed it
	// TODO now that -RTCc has just been deprecated entirely, should we switch back?
    float g = ((float) ((BYTE) ((bgcolorref & 0xFF00) >> 8))) / 255.0;
	float b = ((float) GetBValue(bgcolorref)) / 255.0;

    glClearColor(r, g, b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

HRESULT uiAreaOpenGLEndDraw(uiArea * a) {
	SwapBuffers(a->hDC);
	return S_OK;
}

void uiAreaOpenGLUninit(uiArea * a) {
    if (a->hglrc) {
        wglMakeCurrent(NULL, NULL);
        ReleaseDC(a->hwnd, a->hDC);
        wglDeleteContext(a->hglrc);
        a->hDC = NULL;
        a->hglrc = NULL;
    }
}
