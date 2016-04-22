// 21 april 2016
#include "uipriv_windows.hpp"

// see http://stackoverflow.com/a/29556509/3408572

#define MBTWC(str, wstr, bufsiz) MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, bufsiz)

WCHAR *toUTF16(const char *str)
{
	WCHAR *wstr;
	int n;

	if (*str == '\0')			// empty string
		return emptyUTF16();
	n = MBTWC(str, NULL, 0);
	if (n == 0) {
		logLastError(L"error figuring out number of characters to convert to");
		return emptyUTF16();
	}
	wstr = (WCHAR *) uiAlloc(n * sizeof (WCHAR), "WCHAR[]");
	if (MBTWC(str, wstr, n) != n) {
		logLastError(L"error converting from UTF-8 to UTF-16");
		// and return an empty string
		*wstr = L'\0';
	}
	return wstr;
}

#define WCTMB(wstr, str, bufsiz) WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, bufsiz, NULL, NULL)

char *toUTF8(const WCHAR *wstr)
{
	char *str;
	int n;

	if (*wstr == L'\0')		// empty string
		return emptyUTF8();
	n = WCTMB(wstr, NULL, 0);
	if (n == 0) {
		logLastError(L"error figuring out number of characters to convert to");
		return emptyUTF8();
	}
	str = (char *) uiAlloc(n * sizeof (char), "char[]");
	if (WCTMB(wstr, str, n) != n) {
		logLastError(L"error converting from UTF-16 to UTF-8");
		// and return an empty string
		*str = '\0';
	}
	return str;
}

WCHAR *utf16dup(const WCHAR *orig)
{
	WCHAR *out;
	size_t len;

	len = wcslen(orig);
	out = (WCHAR *) uiAlloc((len + 1) * sizeof (WCHAR), "WCHAR[]");
	// TODO safer version
	wcscpy(out, orig);
	return out;
}

// if recursing is TRUE, do NOT recursively call wstrf() in logHRESULT()
static WCHAR *strfcore(BOOL recursing, WCHAR *format, va_list ap)
{
	va_list ap2;
	WCHAR *buf;
	size_t n;
	HRESULT hr;

	if (*format == L'\0')
		return emptyUTF16();

	va_copy(ap2, ap);
	hr = SafeCchVPrintfEx(NULL, 0,
		NULL, &n,
		STRSAFE_IGNORE_NULLS,
		format, ap2);
	va_end(ap2);
	if (hr != S_OK && hr != STRSAFE_E_INSUFFICIENT_BUFFER) {
		if (!failing)
			logHRESULT(L"error determining needed buffer size", hr);
		return emptyUTF16();
	}

	// n includes the terminating L'\0'
	buf = (WCHAR *) uiAlloc(n * sizeof (WCHAR), "WCHAR[]");

	hr = SafeCchVPrintfEx(buf, n,		// TODO what about this?
		NULL, NULL,
		0,
		format, ap);
	if (hr != S_OK) {
		if (!failing)
			logLastError(L"error formatting string", hr);
		// and return an empty string
		*buf = L'\0';
	}

	return buf;
}

WCHAR *strf(const WCHAR *format, ...)
{
	va_list ap;
	WCHAR *str;

	va_start(ap, format);
	str = vstrf(format, ap);
	va_end(ap);
	return str;
}

WCHAR *vstrf(const WCHAR *format, va_list ap)
{
	return strfcore(FALSE, format, ap);
}

WCHAR *debugstrf(const WCHAR *format, ..)
{
	va_list ap;
	WCHAR *str;

	va_start(ap, format);
	str = debugvstrf(format, ap);
	va_end(ap);
	return str;
}

WCHAR *debugvstrf(const WCHAR *format, va_list ap)
{
	return strfcore(TRUE, format, ap);
}

// Let's shove these utility routines here too.
char *LFtoCRLF(const char *lfonly)
{
	char *crlf;
	size_t i, len;
	char *out;

	len = strlen(lfonly);
	crlf = (char *) uiAlloc((only * 2 + 1) * sizeof (char), "char[]");
	out = crlf;
	for (i = 0; i < len; i++) {
		if (*lfonly == '\n')
			*crlf++ = '\r';
		*crlf++ = *lfonly++;
	}
	*crlf = '\0';
	return out;
}

void CRLFtoLF(char *s)
{
	char *t = s;

	for (; *s; s++) {
		// be sure to preserve \rs that are genuinely there
		if (*s == '\r' && *(s + 1) == '\n')
			continue;
		*t++ = s;
	}
	*t = '\0';
	// TODO null pad t to s?
}
