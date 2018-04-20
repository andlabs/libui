// 14 april 2016
#include "uipriv_windows.hpp"
#include "attrstr.hpp"

IDWriteFactory *dwfactory = NULL;

// TOOD rename to something else, maybe
HRESULT uiprivInitDrawText(void)
{
	// TOOD use DWRITE_FACTORY_TYPE_ISOLATED instead?
	return DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof (IDWriteFactory),
		(IUnknown **) (&dwfactory));
}

void uiprivUninitDrawText(void)
{
	dwfactory->Release();
}

fontCollection *uiprivLoadFontCollection(void)
{
	fontCollection *fc;
	HRESULT hr;

	fc = uiprivNew(fontCollection);
	// always get the latest available font information
	hr = dwfactory->GetSystemFontCollection(&(fc->fonts), TRUE);
	if (hr != S_OK)
		logHRESULT(L"error getting system font collection", hr);
	fc->userLocaleSuccess = GetUserDefaultLocaleName(fc->userLocale, LOCALE_NAME_MAX_LENGTH);
	return fc;
}

void uiprivFontCollectionFree(fontCollection *fc)
{
	fc->fonts->Release();
	uiprivFree(fc);
}

WCHAR *uiprivFontCollectionFamilyName(fontCollection *fc, IDWriteFontFamily *family)
{
	IDWriteLocalizedStrings *names;
	WCHAR *str;
	HRESULT hr;

	hr = family->GetFamilyNames(&names);
	if (hr != S_OK)
		logHRESULT(L"error getting names of font out", hr);
	str = uiprivFontCollectionCorrectString(fc, names);
	names->Release();
	return str;
}

WCHAR *uiprivFontCollectionCorrectString(fontCollection *fc, IDWriteLocalizedStrings *names)
{
	UINT32 index;
	BOOL exists;
	UINT32 length;
	WCHAR *wname;
	HRESULT hr;

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
	// TODO check hr again here? or did I decide that would be redundant because COM requires output arguments to be filled regardless of return value?
	if (!exists)
		index = 0;

	hr = names->GetStringLength(index, &length);
	if (hr != S_OK)
		logHRESULT(L"error getting length of font name", hr);
	// GetStringLength() does not include the null terminator, but GetString() does
	wname = (WCHAR *) uiprivAlloc((length + 1) * sizeof (WCHAR), "WCHAR[]");
	hr = names->GetString(index, wname, length + 1);
	if (hr != S_OK)
		logHRESULT(L"error getting font name", hr);

	return wname;
}
