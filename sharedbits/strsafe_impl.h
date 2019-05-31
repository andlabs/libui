// 31 may 2019
// only requires strsafe_header.h if you don't define sharedbitsStatic as static

#include "start.h"

#ifdef sharedbitsStatic
sharedBitsStatic
#else
extern
#endif
void sharedbitsPrefixName(InternalError)(const char *fmt, ...);

#ifdef sharedbitsStatic
sharedbitsStatic
#endif
int sharedbitsPrefixName(Vsnprintf)(char *s, size_t n, const char *fmt, va_list ap)
{
#ifdef _WIN32
	int ret;

	if (s == NULL && n == 0)
		return _vscprintf(fmt, ap);
	// TODO figure out how to disambiguate between encoding errors (returns negative value; does not have documented errno values), other errors (returns negative value; errno == EINVAL), and truncations (returns -1; does not have documented errno values)
	ret = vsnprintf_s(s, n, _TRUNCATE, fmt, ap);
	if (ret == -1)
		// TODO make this safe
		return (int) n;
	return ret;
#else
	return vsnprintf(s, n, fmt, ap);
#endif
}

#ifdef sharedbitsStatic
sharedbitsStatic
#endif
char *sharedbitsPrefixName(Strncpy)(char *dest, const char *src, size_t n)
{
#ifdef _WIN32
	errno_t err;

	// because strncpy_s() doesn't do this
	memset(dest, '\0', n * sizeof (char));
	err = strncpy_s(dest, n, src, _TRUNCATE);
	if (err != 0 && err != STRUNCATE)
		sharedbitsStaticName(InternalError)("error calling strncpy_s(): %s (%d)", strerror(err), err);
	return dest;
#else
	return strncpy(dest, src, n);
#endif
}

#include "end.h"
