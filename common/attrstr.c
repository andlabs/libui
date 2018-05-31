// 3 december 2016
#include "../ui.h"
#include "uipriv.h"
#include "attrstr.h"

struct uiAttributedString {
	char *s;
	size_t len;

	uiprivAttrList *attrs;

	// indiscriminately keep a UTF-16 copy of the string on all platforms so we can hand this off to the grapheme calculator
	// this ensures no one platform has a speed advantage (sorry GTK+)
	uint16_t *u16;
	size_t u16len;

	size_t *u8tou16;
	size_t *u16tou8;

	// this is lazily created to keep things from getting *too* slow
	uiprivGraphemes *graphemes;
};

static void resize(uiAttributedString *s, size_t u8, size_t u16)
{
	s->len = u8;
	s->s = (char *) uiprivRealloc(s->s, (s->len + 1) * sizeof (char), "char[] (uiAttributedString)");
	s->u8tou16 = (size_t *) uiprivRealloc(s->u8tou16, (s->len + 1) * sizeof (size_t), "size_t[] (uiAttributedString)");
	s->u16len = u16;
	s->u16 = (uint16_t *) uiprivRealloc(s->u16, (s->u16len + 1) * sizeof (uint16_t), "uint16_t[] (uiAttributedString)");
	s->u16tou8 = (size_t *) uiprivRealloc(s->u16tou8, (s->u16len + 1) * sizeof (size_t), "size_t[] (uiAttributedString)");
}

uiAttributedString *uiNewAttributedString(const char *initialString)
{
	uiAttributedString *s;

	s = uiprivNew(uiAttributedString);
	s->attrs = uiprivNewAttrList();
	uiAttributedStringAppendUnattributed(s, initialString);
	return s;
}

// TODO make sure that all implementations of uiprivNewGraphemes() work fine with empty strings; in particular, the Windows one might not
static void recomputeGraphemes(uiAttributedString *s)
{
	if (s->graphemes != NULL)
		return;
	if (uiprivGraphemesTakesUTF16()) {
		s->graphemes = uiprivNewGraphemes(s->u16, s->u16len);
		return;
	}
	s->graphemes = uiprivNewGraphemes(s->s, s->len);
}

static void invalidateGraphemes(uiAttributedString *s)
{
	if (s->graphemes == NULL)
		return;
	uiprivFree(s->graphemes->pointsToGraphemes);
	uiprivFree(s->graphemes->graphemesToPoints);
	uiprivFree(s->graphemes);
	s->graphemes = NULL;
}

void uiFreeAttributedString(uiAttributedString *s)
{
	uiprivFreeAttrList(s->attrs);
	invalidateGraphemes(s);
	uiprivFree(s->u16tou8);
	uiprivFree(s->u8tou16);
	uiprivFree(s->u16);
	uiprivFree(s->s);
	uiprivFree(s);
}

const char *uiAttributedStringString(const uiAttributedString *s)
{
	return s->s;
}

size_t uiAttributedStringLen(const uiAttributedString *s)
{
	return s->len;
}

static void u8u16len(const char *str, size_t *n8, size_t *n16)
{
	uint32_t rune;
	char buf[4];
	uint16_t buf16[2];

	*n8 = 0;
	*n16 = 0;
	while (*str) {
		str = uiprivUTF8DecodeRune(str, 0, &rune);
		// TODO document the use of the function vs a pointer subtract here
		// TODO also we need to consider namespace collision with utf.h...
		*n8 += uiprivUTF8EncodeRune(rune, buf);
		*n16 += uiprivUTF16EncodeRune(rune, buf16);
	}
}

void uiAttributedStringAppendUnattributed(uiAttributedString *s, const char *str)
{
	uiAttributedStringInsertAtUnattributed(s, str, s->len);
}

// this works (and returns true, which is what we want) at s->len too because s->s[s->len] is always going to be 0 due to us allocating s->len + 1 bytes and because uiprivRealloc() always zero-fills allocated memory
static int onCodepointBoundary(uiAttributedString *s, size_t at)
{
	uint8_t c;

	// for uiNewAttributedString()
	if (s->s == NULL && at == 0)
		return 1;
	c = (uint8_t) (s->s[at]);
	return c < 0x80 || c >= 0xC0;
}

// TODO note that at must be on a codeoint boundary
void uiAttributedStringInsertAtUnattributed(uiAttributedString *s, const char *str, size_t at)
{
	uint32_t rune;
	char buf[4];
	uint16_t buf16[2];
	size_t n8, n16;		// TODO make loop-local? to avoid using them in the wrong place again
	size_t old, old16;
	size_t oldn8, oldn16;
	size_t oldlen, old16len;
	size_t at16;
	size_t i;

	if (!onCodepointBoundary(s, at)) {
		// TODO
	}

	at16 = 0;
	if (s->u8tou16 != NULL)
		at16 = s->u8tou16[at];

	// do this first to reclaim memory
	invalidateGraphemes(s);

	// first figure out how much we need to grow by
	// this includes post-validated UTF-8
	u8u16len(str, &n8, &n16);

	// and resize
	old = at;
	old16 = at16;
	oldlen = s->len;
	old16len = s->u16len;
	resize(s, s->len + n8, s->u16len + n16);

	// move existing characters out of the way
	// note the use of memmove(): https://twitter.com/rob_pike/status/737797688217894912
	memmove(
		s->s + at + n8,
		s->s + at,
		(oldlen - at) * sizeof (char));
	memmove(
		s->u16 + at16 + n16,
		s->u16 + at16,
		(old16len - at16) * sizeof (uint16_t));
	// note the + 1 for these; we want to copy the terminating null too
	memmove(
		s->u8tou16 + at + n8,
		s->u8tou16 + at,
		(oldlen - at + 1) * sizeof (size_t));
	memmove(
		s->u16tou8 + at16 + n16,
		s->u16tou8 + at16,
		(old16len - at16 + 1) * sizeof (size_t));
	oldn8 = n8;
	oldn16 = n16;

	// and copy
	while (*str) {
		size_t n;

		str = uiprivUTF8DecodeRune(str, 0, &rune);
		n = uiprivUTF8EncodeRune(rune, buf);
		n16 = uiprivUTF16EncodeRune(rune, buf16);
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
	// TODO is this done by the below?
//TODO	s->u8tou16[old] = old16;
//TODO	s->u16tou8[old16] = old;

	// and adjust the prior values in the conversion tables
	// use <= so the terminating 0 gets updated too
	for (i = 0; i <= oldlen - at; i++)
		s->u8tou16[at + oldn8 + i] += s->u16len - old16len;
	for (i = 0; i <= old16len - at16; i++)
		s->u16tou8[at16 + oldn16 + i] += s->len - oldlen;

	// and finally do the attributes
	uiprivAttrListInsertCharactersUnattributed(s->attrs, at, n8);
}

// TODO document that end is the first index that will be maintained
void uiAttributedStringDelete(uiAttributedString *s, size_t start, size_t end)
{
	size_t start16, end16;
	size_t count, count16;
	size_t i;

	if (!onCodepointBoundary(s, start)) {
		// TODO
	}
	if (!onCodepointBoundary(s, end)) {
		// TODO
	}

	count = end - start;
	start16 = s->u8tou16[start];
	end16 = s->u8tou16[end];
	count16 = end16 - start16;

	invalidateGraphemes(s);

	// overwrite old characters
	memmove(
		s->s + start,
		s->s + end,
		(s->len - end) * sizeof (char));
	memmove(
		s->u16 + start16,
		s->u16 + end16,
		(s->u16len - end16) * sizeof (uint16_t));
	// note the + 1 for these; we want to copy the terminating null too
	memmove(
		s->u8tou16 + start,
		s->u8tou16 + end,
		(s->len - end + 1) * sizeof (size_t));
	memmove(
		s->u16tou8 + start16,
		s->u16tou8 + end16,
		(s->u16len - end16 + 1) * sizeof (size_t));

	// update the conversion tables
	// note the use of <= to include the null terminator
	for (i = 0; i <= (s->len - end); i++)
		s->u8tou16[start + i] -= count16;
	for (i = 0; i <= (s->u16len - end16); i++)
		s->u16tou8[start16 + i] -= count;

	// null-terminate the string
	s->s[start + (s->len - end)] = 0;
	s->u16[start16 + (s->u16len - end16)] = 0;

	// fix up attributes
	uiprivAttrListRemoveCharacters(s->attrs, start, end);

	// and finally resize
	resize(s, s->len - count, s->u16len - count16);
}

void uiAttributedStringSetAttribute(uiAttributedString *s, uiAttribute *a, size_t start, size_t end)
{
	uiprivAttrListInsertAttribute(s->attrs, a, start, end);
}

// LONGTERM introduce an iterator object instead?
void uiAttributedStringForEachAttribute(const uiAttributedString *s, uiAttributedStringForEachAttributeFunc f, void *data)
{
	uiprivAttrListForEach(s->attrs, s, f, data);
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
	if (uiprivGraphemesTakesUTF16())
		pos = s->u8tou16[pos];
	return s->graphemes->pointsToGraphemes[pos];
}

size_t uiAttributedStringGraphemeToByteIndex(uiAttributedString *s, size_t pos)
{
	recomputeGraphemes(s);
	pos = s->graphemes->graphemesToPoints[pos];
	if (uiprivGraphemesTakesUTF16())
		pos = s->u16tou8[pos];
	return pos;
}

// helpers for platform-specific code

const uint16_t *uiprivAttributedStringUTF16String(const uiAttributedString *s)
{
	return s->u16;
}

size_t uiprivAttributedStringUTF16Len(const uiAttributedString *s)
{
	return s->u16len;
}

// TODO is this still needed given the below?
size_t uiprivAttributedStringUTF8ToUTF16(const uiAttributedString *s, size_t n)
{
	return s->u8tou16[n];
}

size_t *uiprivAttributedStringCopyUTF8ToUTF16Table(const uiAttributedString *s, size_t *n)
{
	size_t *out;
	size_t nbytes;

	nbytes = (s->len + 1) * sizeof (size_t);
	*n = s->len;
	out = (size_t *) uiprivAlloc(nbytes, "size_t[] (uiAttributedString)");
	memmove(out, s->u8tou16, nbytes);
	return out;
}

size_t *uiprivAttributedStringCopyUTF16ToUTF8Table(const uiAttributedString *s, size_t *n)
{
	size_t *out;
	size_t nbytes;

	nbytes = (s->u16len + 1) * sizeof (size_t);
	*n = s->u16len;
	out = (size_t *) uiprivAlloc(nbytes, "size_t[] (uiAttributedString)");
	memmove(out, s->u16tou8, nbytes);
	return out;
}
