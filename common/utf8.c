// 17 may 2020
#include "uipriv.h"
#include "third_party/utf.h"

// TODO write separate tests for this file?
// TODO ideally this functionality should really be part of utf itself, in some form or another (for instance, via utf8SanitizedLen() + the requisite loop)

#define nGrow 32

char *uiprivSanitizeUTF8(const char *str)
{
	size_t len;
	char *out;
	const char *s;
	size_t i;
	uint32_t rune;
	char encoded[4];
	size_t n;

	// TODO can we even use strlen() with UTF-8 strings? or is '\0' == 0 == actual memory zero just a source code connection (and thus the last one isn't necessarily true)?
	len = strlen(str);
	out = (char *) uiprivAlloc((len + 1) * sizeof (char), "sanitized UTF-8 string");
	s = str;
	i = 0;
	while (*s != '\0') {
		s = uiprivUTF8DecodeRune(s, 0, &rune);
		n = uiprivUTF8EncodeRune(rune, encoded);
		if ((i + n) >= len) {
			out = (char *) uiprivRealloc(out, (len + 1) * sizeof (char), (len + nGrow + 1) * sizeof (char), "sanitized UTF-8 string");
			len += nGrow;
		}
		memcpy(out + i, encoded, n);
		i += n;
	}
	return out;
}

void uiprivFreeUTF8(char *sanitized)
{
	uiprivFree(sanitized);
}
