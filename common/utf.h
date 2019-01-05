// utf by pietro gagliardi (andlabs) — https://github.com/andlabs/utf/
// 10 november 2016

// note the overridden names with uipriv at the beginning; this avoids potential symbol clashes when building libui as a static library
// LONGTERM find a way to encode the name overrides directly into the utf library

#ifdef __cplusplus
extern "C" {
#endif

// TODO (for utf itself as well) should this go outside the extern "C" block or not
#include <stddef.h>
#include <stdint.h>

// if nElem == 0, assume the buffer has no upper limit and is '\0' terminated
// otherwise, assume buffer is NOT '\0' terminated but is bounded by nElem *elements*

extern size_t uiprivUTF8EncodeRune(uint32_t rune, char *encoded);
extern const char *uiprivUTF8DecodeRune(const char *s, size_t nElem, uint32_t *rune);
extern size_t uiprivUTF16EncodeRune(uint32_t rune, uint16_t *encoded);
extern const uint16_t *uiprivUTF16DecodeRune(const uint16_t *s, size_t nElem, uint32_t *rune);

extern size_t uiprivUTF8RuneCount(const char *s, size_t nElem);
extern size_t uiprivUTF8UTF16Count(const char *s, size_t nElem);
extern size_t uiprivUTF16RuneCount(const uint16_t *s, size_t nElem);
extern size_t uiprivUTF16UTF8Count(const uint16_t *s, size_t nElem);

#ifdef __cplusplus
}

// TODO sync this back to upstream (need copyright clearance first)

// On Windows, wchar_t is equivalent to uint16_t, but C++ requires
// wchar_t to be a completely distinct type. These overloads allow
// passing wchar_t pointers directly into these functions from C++
// on Windows. Otherwise, you'd need to cast to pass a wchar_t
// pointer, WCHAR pointer, or equivalent to these functions.
// 
// This does not apply to MSVC because the situation there is
// slightly more complicated; see below.
#if defined(_WIN32) && !defined(_MSC_VER)

inline size_t uiprivUTF16EncodeRune(uint32_t rune, wchar_t *encoded)
{
	return uiprivUTF16EncodeRune(rune, reinterpret_cast<uint16_t *>(encoded));
}

inline const wchar_t *uiprivUTF16DecodeRune(const wchar_t *s, size_t nElem, uint32_t *rune)
{
	const uint16_t *ret;

	ret = uiprivUTF16DecodeRune(reinterpret_cast<const uint16_t *>(s), nElem, rune);
	return reinterpret_cast<const wchar_t *>(ret);
}

inline size_t uiprivUTF16RuneCount(const wchar_t *s, size_t nElem)
{
	return uiprivUTF16RuneCount(reinterpret_cast<const uint16_t *>(s), nElem);
}

inline size_t uiprivUTF16UTF8Count(const wchar_t *s, size_t nElem)
{
	return uiprivUTF16UTF8Count(reinterpret_cast<const uint16_t *>(s), nElem);
}

#endif

// This is the same as the above, except that with MSVC, whether
// wchar_t is a keyword or not is controlled by a compiler option!
// (At least with gcc, this is not the case; thanks redi in
// irc.freenode.net/#gcc.) We use __wchar_t to be independent of
// the option; see https://blogs.msdn.microsoft.com/oldnewthing/20161201-00/?p=94836
// (ironically posted one day after I initially wrote this code!).
// TODO should defined(_WIN32) be used too?
// TODO check this under /Wall
// TODO are C-style casts enough? or will that fail in /Wall?
// TODO same for UniChar/unichar on Mac? if both are unsigned then we have nothing to worry about
#if defined(_MSC_VER)

inline size_t uiprivUTF16EncodeRune(uint32_t rune, __wchar_t *encoded)
{
	return uiprivUTF16EncodeRune(rune, reinterpret_cast<uint16_t *>(encoded));
}

inline const __wchar_t *uiprivUTF16DecodeRune(const __wchar_t *s, size_t nElem, uint32_t *rune)
{
	const uint16_t *ret;

	ret = uiprivUTF16DecodeRune(reinterpret_cast<const uint16_t *>(s), nElem, rune);
	return reinterpret_cast<const __wchar_t *>(ret);
}

inline size_t uiprivUTF16RuneCount(const __wchar_t *s, size_t nElem)
{
	return uiprivUTF16RuneCount(reinterpret_cast<const uint16_t *>(s), nElem);
}

inline size_t uiprivUTF16UTF8Count(const __wchar_t *s, size_t nElem)
{
	return uiprivUTF16UTF8Count(reinterpret_cast<const uint16_t *>(s), nElem);
}

#endif

#endif
