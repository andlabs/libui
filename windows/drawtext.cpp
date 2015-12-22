// 22 december 2015
// Before we begin, you may be wondering why this file is C++.
// Simple: <dwrite.h> is C++ only! Thanks Microsoft!
// And unlike UI Automation which accidentally just forgets the 'struct' and 'enum' tags in places, <dwrite.h> is a full C++ header file, with class definitions and the use of __uuidof. Oh well :/
#include "uipriv_windows.h"

static IDWriteFactory *dwfactory = NULL;

HRESULT initDrawText(void)
{
	// TOOD use DWRITE_FACTORY_TYPE_ISOLATED instead?
	return DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof (IDWriteFactory),
		(IUnknown **) (&dwfactory));
}

void uninitDrawText(void)
{
	dwfactory->Release();
}
