// 14 april 2016
// Before we begin, you may be wondering why this file is C++.
// Simple: <dwrite.h> is C++ only! Thanks Microsoft!
// And unlike UI Automation which accidentally just forgets the 'struct' and 'enum' tags in places, <dwrite.h> is a full C++ header file, with class definitions and the use of __uuidof. Oh well :/
#include "uipriv_windows.h"

IDWriteFactory *dwfactory = NULL;

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

fontCollection *loadFontCollection(void)
{
	fontCollection *fc;
	HRESULT hr;

	fc = uiNew(fontCollection);
	// always get the latest available font information
	hr = dwfactory->GetSystemFontCollection(&(fc->fonts), TRUE);
	if (hr != S_OK)
		logHRESULT("error getting system font collection in loadFontCollection()", hr);
	fc->userLocaleSuccess = GetUserDefaultLocaleName(fc->userLocale, LOCALE_NAME_MAX_LENGTH);
	return fc;
}

WCHAR *fontCollectionFamilyName(fontCollection *fc, IDWriteFontFamily *family)
{
	IDWriteLocalizedStrings *names;
	UINT32 index;
	BOOL exists;
	UINT32 length;
	WCHAR *wname;
	HRESULT hr;

	hr = family->GetFamilyNames(&names);
	if (hr != S_OK)
		logHRESULT("error getting names of font out in fontCollectionFamilyName()", hr);

	// this is complex, but we ignore failure conditions to allow fallbacks
	// 1) If the user locale name was successfully retrieved, try it
	// 2) If the user locale name was not successfully retrieved, or that locale's string does not exist, or an error occurred, try L"en-us", the US English locale
	// 3) And if that fails, assume the first one
	// This algorithm is straight from MSDN: https://msdn.microsoft.com/en-us/library/windows/desktop/dd368214%28v=vs.85%29.aspx
	// For step 2 to work, start by setting hr to S_OK and exists to FALSE.
	// TODO does it skip step 2 entirely if step 1 fails? rewrite it to be a more pure conversion of the MSDN code?
	hr = S_OK;
	exists = FALSE;
	if (fc->userLocaleSuccess != 0)
		hr = names->FindLocaleName(fc->userLocale, &index, &exists);
	if (hr != S_OK || (hr == S_OK && !exists))
		hr = names->FindLocaleName(L"en-us", &index, &exists);
	if (!exists)
		index = 0;

	hr = names->GetStringLength(index, &length);
	if (hr != S_OK)
		logHRESULT("error getting length of font name in fontCollectionFamilyName()", hr);
	// GetStringLength() does not include the null terminator, but GetString() does
	wname = (WCHAR *) uiAlloc((length + 1) * sizeof (WCHAR), "WCHAR[]");
	hr = names->GetString(index, wname, length + 1);
	if (hr != S_OK)
		logHRESULT("error getting font name in fontCollectionFamilyName()", hr);

	names->Release();
	return wname;
}

void fontCollectionFree(fontCollection *fc)
{
	fc->fonts->Release();
	uiFree(fc);
}
