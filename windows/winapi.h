// 31 may 2015
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define CINTERFACE
#define COBJMACROS
// see https://github.com/golang/go/issues/9916#issuecomment-74812211
#define INITGUID
#define _USE_MATH_DEFINES
// get Windows version right; right now Windows Vista
#define WINVER 0x0600				/* according to Microsoft's winnls.h */
#define _WIN32_WINNT 0x0600		/* according to Microsoft's sdkddkver.h */
#define _WIN32_WINDOWS 0x0600		/* according to Microsoft's pdh.h */
#define _WIN32_IE 0x0700			/* according to Microsoft's sdkddkver.h */
#define NTDDI_VERSION 0x06000000	/* according to Microsoft's sdkddkver.h */
#include <windows.h>
#include <commctrl.h>
#include <stdint.h>
#include <uxtheme.h>
#include <string.h>
#include <wchar.h>
#include <windowsx.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <stdarg.h>
#include <oleacc.h>
#include <stdio.h>
#include <shobjidl.h>
#include <math.h>
#include <d2d1.h>
