// 31 may 2019
// only requires strsafe_header.h if you don't define sharedbitsStatic as static

#include "start.h"

#ifdef _WIN32
#ifdef sharedbitsInternalError
#define sharedbitsprivInternalError sharedbitsInternalError
#else
#define sharedbitsprivInternalError sharedbitsPrefixName(InternalError)
#ifdef sharedbitsStatic
sharedbitsStatic
#else
extern
#endif
void sharedbitsprivInternalError(const char *fmt, ...);
#endif
#endif

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
		// Yes folks, apparently strerror() is unsafe (it's not reentrant, but that's not the point of the MSVC security functions; that's about buffer overflows, and as you'll soon see there really is no need for what the "safe' version is given reentrancy concerns), and not only that, but the replacement, strerror_s(), requires copying and allocation! it's almost like they were TRYING to shove as many error conditions as possible in!
		// Oh, and you can't just use _sys_errlist[] to bypass this, because even that has a deprecation warning, telling you to use strerror() instead, which in turn sends you back to strerror_s()!
		// Of course, the fact _sys_errlist[] is a thing and that it's deprecated out of security and not reentrancy shows that the error strings returned by strerror()/strerror_s() are static and unchanging throughout the lifetime of the program, so a truly reentrant strerror_s() would just return the raw const string array directly, or a placeholder like "unknown error" otherwise, but that would be too easy!
		// And even better, there's no way to get the length of the error message, so you can't even dynamically allocate a large enough buffer if you wanted to!
		// (Furthermore, cppreference.com says there's strerrorlen_s(), but a) fuck C11, and b) MSDN does not concur.)
		// So, alas, you'll have to live with just having the error code; sorry.
		sharedbitsprivInternalError("error calling strncpy_s(): %d", err);
	return dest;
#else
	return strncpy(dest, src, n);
#endif
}

#undef sharedbitsprivInternalError

#include "end.h"
