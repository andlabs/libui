// 31 may 2019
// only requires strsafe_header.h if you don't define sharedbitsStatic as static

#include "start.h"

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

#include "end.h"

#include "strsafe_strncpy_impl.h"
