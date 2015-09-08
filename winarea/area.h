// 8 september 2015
#include "../windows/winapi.h"
#include <stdint.h>
#include "ui.h"

extern HINSTANCE hInstance;

extern ATOM registerAreaClass(void);
extern void unregisterAreaClass(void);
extern HWND makeArea(DWORD exstyle, DWORD style, int x, int y, int cx, int cy, HWND parent, uiAreaHandler *ah);

extern HRESULT logLastError(const char *);
extern HRESULT logHRESULT(const char *, HRESULT);
extern HRESULT logMemoryExhausted(const char *);

extern uiDrawContext *newContext(HDC);
