// 3 december 2016
#include "../ui.h"
#include "uipriv.h"

struct uiAttributedString {
	char *s;
	size_t len;

	// TODO attributes

	// indiscriminately keep a UTF-16 copy of the string on all platforms so we can hand this off to the grapheme calculator
	// this ensures no one platform has a speed advantage (sorry GTK+)
	uint16_t *u16;
	size_t u16len;

	size_t *u8tou16;
	size_t *u16tou8;

	// this is lazily created to keep things from getting *too* slow
	struct graphemes *graphemes;
};

static void resize(uiAttributedString *s, size_t u8, size_t u16)
{
	s->len = u8;
	s->s = (char *) uiRealloc(s->s, (s->len + 1) * sizeof (char), "char[] (uiAttributedString)");
	s->u8tou16 = (size_t *) uiRealloc(s->u8tou16, (s->len + 1) * sizeof (size_t), "size_t[] (uiAttributedString)");
	s->u16len = u16;
	s->u16 = (uint16_t *) uiRealloc(s->u16, (s->u16len + 1) * sizeof (uint16_t), "uint16_t[] (uiAttributedString)");
	s->u16tou8 = (size_t *) uiRealloc(s->u16tou8, (s->u16len + 1) * sizeof (size_t), "size_t[] (uiAttributedString)");
}

uiAttributedString *uiNewAttributedString(const char *initialString)
{
	uiAttributedString *s;

	s = uiNew(uiAttributedString);
	uiAttributedStringAppendUnattributed(s, initialString);
	return s;
}

static void recomputeGraphemes(uiAttributedString *s)
{
	if (s->graphemes != NULL)
		return;
	if (graphemesTakesUTF16()) {
		s->graphemes = graphemes(s->u16, s->u16len);
		return;
	}
	s->graphemes = graphemes(s->s, s->len);
}

static void invalidateGraphemes(uiAttributedString *s)
{
	if (s->graphemes == NULL)
		return;
	uiFree(s->graphemes->pointsToGraphemes);
	uiFree(s->graphemes->graphemesToPoints);
	uiFree(s->graphemes);
	s->graphemes = NULL;
}

void uiFreeAttributedString(uiAttributedString *s)
{
	invalidateGraphemes(s);
	uiFree(s->u16tou8);
	uiFree(s->u8tou16);
	uiFree(s->u16);
	uiFree(s->s);
	uiFree(s);
}

const char *uiAttributedStringString(uiAttributedString *s)
{
	return s->s;
}

void uiAttributedStringAppendUnattributed(uiAttributedString *s, const char *str)
{
	const char *t;
	uint32_t rune;
	char buf[4];
	uint16_t u16buf[2];
	size_t n, n16;
	size_t old, old;

	// first figure out how much we need to grow by
	// this includes post-validated UTF-8
	t = str;
	n = 0;
	n16 = 0;
	while (*t) {
		t = utf8DecodeRune(t, 0, &rune);
		n += utf8EncodeRune(rune, buf);
		n16 += utf16EncodeRune(rune, buf16);
	}

	// and resize
	old = s->len;
	old16 = s->len16;
	resize(s, s->len + n, s->u16len + n16);

	// and copy
	while (*str) {
		str = utf8DecodeRune(str, 0, &rune);
		n = utf8EncodeRune(rune, buf);
		n16 = utf16EncodeRune(rune, buf16);
		s->s[old] = buf[0];
		s->u8tou16[old] = old16;
		if (n > 1) {
			s->s[old + 1] = buf[1];
			s->u8tou16[old + 1] = old16;
		}
		if (n > 2) {
			s->s[old + 2] = buf[2];
			s->u8tou16[old + 2] = old16;
		}
		if (n > 3) {
			s->s[old + 3] = buf[3];
			s->u8tou16[old + 3] = old16;
		}
		s->u16[old16] = buf16[0];
		s->u16tou8[old16] = old;
		if (n16 > 1) {
			s->u16[old16 + 1] = buf16[1];
			s->u16tou8[old16 + 1] = old;
		}
		old += n;
		old16 += n16;
	}
	// and have an index for the end of the string
	s->u8tou16[old] = old16;
	s->u16tou8[old16] = old;

	invalidateGraphemes(s);
}

// TODO figure out if we should count the grapheme past the end
size_t uiAttributedStringNumGraphemes(uiAttributedString *s)
{
	recomputeGraphemes(s);
	return s->graphemes->len;
}

size_t uiAttributedStringByteIndexToGrapheme(uiAttributedString *s, size_t pos)
{
	recomputeGraphemes(s);
	if (graphemesTakesUTF16())
		pos = s->u8tou16[pos];
	return s->graphemes->pointsToGraphemes[pos];
}

size_t uiAttributedStringGraphemeToByteIndex(uiAttributedString *s, size_t pos)
{
	recomputeGraphemes(s);
	pos = s->graphemes->graphemesToPoints[pos];
	if (graphemesTakesUTF16())
		pos = s->u16tou8[pos];
	return pos;
}
