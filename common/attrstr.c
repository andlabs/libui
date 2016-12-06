// 3 december 2016
#include "../ui.h"
#include "uipriv.h"

struct uiAttributedString {
	char *s;
	size_t len;

	size_t nAttrs;		// TODO this needs to be maintained; is it necessary?
	struct attr *attrs;
	struct attr *lastattr;

	// indiscriminately keep a UTF-16 copy of the string on all platforms so we can hand this off to the grapheme calculator
	// this ensures no one platform has a speed advantage (sorry GTK+)
	uint16_t *u16;
	size_t u16len;

	size_t *u8tou16;
	size_t *u16tou8;

	// this is lazily created to keep things from getting *too* slow
	struct graphemes *graphemes;
};

struct attr {
	int type;
	uintptr_t val;
	size_t start;
	size_t end;
	struct attr *next;
};

enum {
	// TODO put attr types here
	nAttrTypes,
};

static int attrHasPos(struct attr *a, size_t pos)
{
	if (pos < a->start)
		return 0;
	return pos < a->end;
}

// returns 1 if there was an intersection and 0 otherwise
static int attrRangeIntersect(struct attr *a, size_t *start, size_t *end)
{
	// is the range outside a entirely?
	if (*start >= a->end)
		return 0;
	if (*end < a->start)
		return 0;

	// okay, so there is an overlap
	// compute the intersection
	if (*start < a->start)
		*start = a->start;
	if (*end > a->end)
		*end = a->end;
	return 1;
}

// returns:
// - 0 if no change needed
// - 1 if the attribute was split
static int attrSplitAt(uiAttributedString *s, struct attr *a, size_t at)
{
	struct attr *b;

	// no splittng needed?
	if (at == a->start)
		return 0;
	if ((at + 1) == a->end)
		return 0;

	b = uiNew(struct attr);
	b->what = a->what;
	b->val = a->val;
	b->start = at;
	b->end = a->end;
	b->next = a->next;

	a->end = at;
	a->next = b;
	if (a == s->lastattr)
		s->lastattr = a->next;
	return 1;
}

// removes attributes without deleting characters
// returns:
// - 0 if the attribute needs to be deleted
// - 1 if the attribute was changed or no change needed
// - 2 if the attribute was split
static int attrDropRange(uiAttributedString *s, struct attr *a, size_t start, size_t end)
{
	struct attr *b;

	if (!attrRangeIntersect(a, &start, &end))
		// out of range; nothing to do
		return 1;

	// just outright delete the attribute?
	if (a->start == start && a->end == end)
		return 0;

	// only chop off the start or end?
	if (a->start == start) {		// chop off the end
		a->end = end;
		return 1;
	}
	if (a->end == end) {			// chop off the start
		a->start = start;
		return 1;
	}

	// we'll need to split the attribute into two
	b = uiNew(struct attr);
	b->what = a->what;
	b->val = a->val;
	b->start = end;
	b->end = a->end;
	b->next = a->next;

	a->end = start;
	a->next = b;
	if (a == s->lastattr)
		s->lastattr = a->next;
	return 2;
}

// removes attributes while deleting characters
// returns:
// - 0 if the attribute needs to be deleted
// - 1 if the attribute was changed or no change needed
// - 2 if the attribute was split
static int attrDeleteRange(uiAttributedString *s, struct attr *a, size_t start, size_t end)
{
	struct attr *b;
	struct attr tmp;
	size_t count, acount;

	if (!attrRangeIntersect(a, &start, &end))
		// out of range; nothing to do
		return 1;

	// just outright delete the attribute?
	if (a->start == start && a->end == end)
		return 0;

	acount = a->end - a->start;
	count = end - start;

	// only the start or end deleted?
	if (a->start == start) {		// start deleted
		a->end = a->start + (acount - count);
		return 1;
	}
	if (a->end == end) {			// end deleted
		a->end = a->start + count;
		return 1;
	}

	// something in the middle deleted
	// we ened to split the attribute into *three*
	// first, split at the start of he deleted range
	tmp.what = a->what;
	tmp.val = a->val;
	tmp.start = start;
	tmp.end = a->end;
	tmp.next = a->next;

	a->end = start;
	a->next = &tmp;

	// now split at the end
	b = uiNew(struct attr);
	b->what = a->what;
	b->val = a->val;
	b->start = end;
	b->end = a->end;
	b->next = tmp.next;

	tmp.end = end;
	tmp.next = b;

	// and now push b back to overwrite the deleted stuff
	a->next = b;
	b->start -= count;
	b->end -= count;
	if (a == s->lastattr)
		s->lastattr = a->next;
	return 2;
}

// returns the attribute to continue with
static struct attr *attrDelete(uiAttributedString *s, struct attr *a, struct attr *prev)
{
	if (a == s->attrs) {
		s->attrs = a->next;
		uiFree(a);
		return s->attrs;
	}
	if (a == s->lastattr)
		s->lastattr = prev;
	prev->next = a->next;
	uiFree(a);
	return prev->next;
}

static void attrAppend(uiAttributedString *s, int type, uintptr_t val, size_t start, size_t end)
{
	struct attr *a;

	a = uiNew(struct attr);
	a->type = type;
	a->val = val;
	a->start = start;
	a->end = end;
	if (s->attrs == NULL) {
		s->attrs = a;
		s->lastattr = a;
		return;
	}
	s->lastattr->next = a;
	s->lastattr = a;
}

// alist should be struct attr *alist[nAttrTypes]
static void attrsGetFor(uiAttributedString *s, struct attr **alist, size_t at)
{
	int i;
	struct attr *a;

	// we want the attributes for at
	// these are the attributes of at - 1
	// if at == 0. then htese are the attributes at 0
	if (at != 0)
		at--;

	// make usre unset attributes are NULL
	for (i = 0; i < nAttrTypes; i++)
		alist[i] = NULL;

	for (a = s->attrs; a != NULL; a = a->next) {
		if (!attrHasPos(a, at))
			continue;
		alist[a->type] = a;
	}
}

// TODO have a routine that prunes overridden attributes for a given character from the list? merge it with the above?

static void attrAdjustPostInsert(uiAttributedString *s, size_t start, size_t n, size_t oldlen)
{
	struct attr *a;

	for (a = s->attrs; a != NULL; a = a->next) {
		size_t astart, aend;
		int splitNeeded;

		// do we need to adjust this, and where?
		astart = start;
		aend = oldlen;
		if (!attrRangeIntersect(a, &astart, &aend))
			continue;

		// if only part of the attribute falls in the moved area, we need to split at the insertion point and adjust both resultant attributes
		// otherwise, we only adjust the original attribute
		// split *before* adjusting so that the split is correct
		splitNeeded = attrSplit(s, a, astart);
		if (a->start >= start)
			a->start += n;
		if (a->end >= end)
			a->end += n;
		if (splitNeeded == 1) {
			a = a->next;
			if (a->start >= start)
				a->start += n;
			if (a->end >= end)
				a->end += n;
		}
	}
}

static void attrAdjustPostDelete(uiAttributedString *s, size_t start, size_t end)
{
	struct attr *a, *prev;

	a = s->attrs;
	prev = NULL;
	while (a != NULL) {
		size_t astart, aend;

		// do we need to adjust this, and where?
		astart = start;
		aend = end;
		if (!attrRangeIntersect(a, &astart, &aend)) {
			prev = a;
			a = a->next;
			continue;
		}

		switch (attrDeleteRange(s, a, astart, aend)) {
		case 0:		// delete
			a = attrDelete(s, a, prev);
			// keep prev unchanged
			break;
		case 2:		// split
			a = a->next;
			// fall through
		case 1:		// existing only needed adjustment
			prev = a;
			a = a->next;
			break;
		}
	}
}

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

// TODO make sure that all implementations of graphemes() work fine with empty strings; in particular, the Windows one might not
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
	struct attr *a, *b;

	a = s->attrs;
	while (a != NULL) {
		b = a->next;
		uiFree(a);
		a = b;
	}
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

size_t uiAttributedStringLen(uiAttributedString *s)
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
		str = utf8DecodeRune(str, 0, &rune);
		*n8 += utf8EncodeRune(rune, buf);
		*n16 += utf16EncodeRune(rune, buf16);
	}
}

void uiAttributedStringAppendUnattributed(uiAttributedString *s, const char *str)
{
	uiAttributedStringInsertAtUnattributed(s, str, s->len);
}

// this works (and returns true, which is what we want) at s->len too because s->s[s->len] is always going to be 0 due to us allocating s->len + 1 bytes and because uiRealloc() always zero-fills allocated memory
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
	uint16_t u16buf[2];
	size_t n8, n16;
	size_t old, old16;
	size_t oldlen, old16len;
	size_t at16;
	size_t i;

	if (!onCodepointBoundary(s, at)) {
		// TODO
	}

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

	// and copy
	while (*str) {
		size_t n;

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
	// TODO is this done by the below?
	s->u8tou16[old] = old16;
	s->u16tou8[old16] = old;

	// and adjust the prior values in the conversion tables
	// use <= so the terminating 0 gets updated too
	for (i = 0; i <= oldlen - at; i++)
		s->u8tou16[at + n8 + i] += n16;
	for (i = 0; i <= old16len - at16; i++)
		s->u16tou8[at16 + n16 + i] += n8;

	// and finally do the attributes
	attrAdjustPostInsert(s, at, n8, oldlen);
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
		(oldlen - end) * sizeof (char));
	memmove(
		s->u16 + start16,
		s->u16 + end16,
		(old16len - end16) * sizeof (uint16_t));
	// note the + 1 for these; we want to copy the terminating null too
	memmove(
		s->u8tou16 + start,
		s->u8tou16 + end,
		(oldlen - end + 1) * sizeof (size_t));
	memmove(
		s->u16tou8 + start16,
		s->u16tou8 + end16,
		(old16len - end16 + 1) * sizeof (size_t));

	// update the conversion tables
	// note the use of <= to include the null terminator
	for (i = 0; i <= count; i++)
		s->u8tou16[start + i] -= count16;
	for (i = 0; i <= count16; i++)
		s->u16tou8[start16 + i] -= count;

	// null-terminate the string
	s->s[start + count] = 0;
	s->u16[start16 + count16] = 0;

	// fix up attributes
	attrAdjustPostDelete(s, start, end);

	// and finally resize
	resize(s, start + count, start16 + count16);
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
