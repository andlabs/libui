// 21 april 2016
#include "uipriv_windows.hpp"

// see http://stackoverflow.com/a/29556509/3408572

WCHAR *toUTF16(const char *str)
{
	WCHAR *wstr;
	WCHAR *wp;
	size_t n;
	uint32_t rune;

	if (*str == '\0')			// empty string
		return emptyUTF16();
	n = uiprivUTF8UTF16Count(str, 0);
	wstr = (WCHAR *) uiprivAlloc((n + 1) * sizeof (WCHAR), "WCHAR[]");
	wp = wstr;
	while (*str) {
		str = uiprivUTF8DecodeRune(str, 0, &rune);
		n = uiprivUTF16EncodeRune(rune, wp);
		wp += n;
	}
	return wstr;
}

char *toUTF8(const WCHAR *wstr)
{
	char *str;
	char *sp;
	size_t n;
	uint32_t rune;

	if (*wstr == L'\0')		// empty string
		return emptyUTF8();
	n = uiprivUTF16UTF8Count(wstr, 0);
	str = (char *) uiprivAlloc((n + 1) * sizeof (char), "char[]");
	sp = str;
	while (*wstr) {
		wstr = uiprivUTF16DecodeRune(wstr, 0, &rune);
		n = uiprivUTF8EncodeRune(rune, sp);
		sp += n;
	}
	return str;
}

WCHAR *utf16dup(const WCHAR *orig)
{
	WCHAR *out;
	size_t len;

	len = wcslen(orig);
	out = (WCHAR *) uiprivAlloc((len + 1) * sizeof (WCHAR), "WCHAR[]");
	wcscpy_s(out, len + 1, orig);
	return out;
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
	va_list ap2;
	WCHAR *buf;
	size_t n;

	if (*format == L'\0')
		return emptyUTF16();

	va_copy(ap2, ap);
	n = _vscwprintf(format, ap2);
	va_end(ap2);
	n++;		// terminating L'\0'

	buf = (WCHAR *) uiprivAlloc(n * sizeof (WCHAR), "WCHAR[]");
	// includes terminating L'\0' according to example in https://msdn.microsoft.com/en-us/library/xa1a1a6z.aspx
	vswprintf_s(buf, n, format, ap);

	return buf;
}

// TODO merge the following two with the toUTF*()s?

// Let's shove these utility routines here too.
// Prerequisite: lfonly is UTF-8.
char *LFtoCRLF(const char *lfonly)
{
	char *crlf;
	size_t i, len;
	char *out;

	len = strlen(lfonly);
	crlf = (char *) uiprivAlloc((len * 2 + 1) * sizeof (char), "char[]");
	out = crlf;
	for (i = 0; i < len; i++) {
		if (*lfonly == '\n')
			*crlf++ = '\r';
		*crlf++ = *lfonly++;
	}
	*crlf = '\0';
	return out;
}

// Prerequisite: s is UTF-8.
void CRLFtoLF(char *s)
{
	char *t = s;

	for (; *s != '\0'; s++) {
		// be sure to preserve \rs that are genuinely there
		if (*s == '\r' && *(s + 1) == '\n')
			continue;
		*t++ = *s;
	}
	*t = '\0';
	// pad out the rest of t, just to be safe
	while (t != s)
		*t++ = '\0';
}

// std::to_string() always uses %f; we want %g
// fortunately std::iostream seems to use %g by default so
WCHAR *ftoutf16(double d)
{
	std::wostringstream ss;
	std::wstring s;

	ss << d;
	s = ss.str();		// to be safe
	return utf16dup(s.c_str());
}

// to complement the above
WCHAR *itoutf16(int i)
{
	std::wostringstream ss;
	std::wstring s;

	ss << i;
	s = ss.str();		// to be safe
	return utf16dup(s.c_str());
}
