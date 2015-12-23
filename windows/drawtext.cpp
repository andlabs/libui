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

struct uiDrawFontFamilies {
	IDWriteFontCollection *fonts;
	WCHAR userLocale[LOCALE_NAME_MAX_LENGTH];
	int userLocaleSuccess;
};

uiDrawFontFamilies *uiDrawListFontFamilies(void)
{
	uiDrawFontFamilies *ff;
	HRESULT hr;

	ff = uiNew(uiDrawFontFamilies);
	// always get the latest available font information
	hr = dwfactory->GetSystemFontCollection(&(ff->fonts), TRUE);
	if (hr != S_OK)
		logHRESULT("error getting list of system fonts in uiDrawListFontFamilies()", hr);
	ff->userLocaleSuccess = GetUserDefaultLocaleName(ff->userLocale, LOCALE_NAME_MAX_LENGTH);
	return ff;
}

uintmax_t uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return ff->fonts->GetFontFamilyCount();
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, uintmax_t n)
{
	IDWriteFontFamily *family;
	IDWriteLocalizedStrings *names;
	UINT32 index;
	BOOL exists;
	UINT32 length;
	WCHAR *wname;
	char *name;
	HRESULT hr;

	hr = ff->fonts->GetFontFamily(n, &family);
	if (hr != S_OK)
		logHRESULT("error getting font out of collection in uiDrawFontFamiliesFamily()", hr);
	hr = family->GetFamilyNames(&names);
	if (hr != S_OK)
		logHRESULT("error getting names of font out in uiDrawFontFamiliesFamily()", hr);

	// this is complex, but we ignore failure conditions to allow fallbacks
	// 1) If the user locale name was successfully retrieved, try it
	// 2) If the user locale name was not successfully retrieved, or that locale's string does not exist, or an error occurred, try L"en-us", the US English locale
	// 3) And if that fails, assume the first one
	// This algorithm is straight from MSDN: https://msdn.microsoft.com/en-us/library/windows/desktop/dd368214%28v=vs.85%29.aspx
	// For step 2 to work, start by setting hr to S_OK and exists to FALSE.
	// TODO does it skip step 2 entirely if step 1 fails? rewrite it to be a more pure conversion of the MSDN code?
	hr = S_OK;
	exists = FALSE;
	if (ff->userLocaleSuccess != 0)
		hr = names->FindLocaleName(ff->userLocale, &index, &exists);
	if (hr != S_OK || (hr == S_OK && !exists))
		hr = names->FindLocaleName(L"en-us", &index, &exists);
	if (!exists)
		index = 0;

	hr = names->GetStringLength(index, &length);
	if (hr != S_OK)
		logHRESULT("error getting length of font name in uiDrawFontFamiliesFamily()", hr);
	// GetStringLength() does not include the null terminator, but GetString() does
	wname = (WCHAR *) uiAlloc((length + 1) * sizeof (WCHAR), "WCHAR[]");
	hr = names->GetString(index, wname, length + 1);
	if (hr != S_OK)
		logHRESULT("error getting font name in uiDrawFontFamiliesFamily()", hr);

	name = toUTF8(wname);

	uiFree(wname);
	names->Release();
	family->Release();
	return name;
}

void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff)
{
	ff->fonts->Release();
	uiFree(ff);
}
