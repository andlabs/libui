// utf by pietro gagliardi (andlabs) — https://github.com/andlabs/utf/
// 10 november 2016
// function names have been altered to avoid namespace collisions in libui static builds (see utf.h)
#include "utf.h"

// this code imitates Go's unicode/utf8 and unicode/utf16
// the biggest difference is that a rune is unsigned instead of signed (because Go guarantees what a right shift on a signed number will do, whereas C does not)
// it is also an imitation so we can license it under looser terms than the Go source
#define badrune 0xFFFD

// encoded must be at most 4 bytes
// TODO clean this code up somehow
size_t uiprivUTF8EncodeRune(uint32_t rune, char *encoded)
{
	uint8_t b, c, d, e;
	size_t n;

	// not in the valid range for Unicode
	if (rune > 0x10FFFF)
		rune = badrune;
	// surrogate runes cannot be encoded
	if (rune >= 0xD800 && rune < 0xE000)
		rune = badrune;

	if (rune < 0x80) {		// ASCII bytes represent themselves
		b = (uint8_t) (rune & 0xFF);
		n = 1;
		goto done;
	}
	if (rune < 0x800) {		// two-byte encoding
		c = (uint8_t) (rune & 0x3F);
		c |= 0x80;
		rune >>= 6;
		b = (uint8_t) (rune & 0x1F);
		b |= 0xC0;
		n = 2;
		goto done;
	}
	if (rune < 0x10000) {	// three-byte encoding
		d = (uint8_t) (rune & 0x3F);
		d |= 0x80;
		rune >>= 6;
		c = (uint8_t) (rune & 0x3F);
		c |= 0x80;
		rune >>= 6;
		b = (uint8_t) (rune & 0x0F);
		b |= 0xE0;
		n = 3;
		goto done;
	}
	// otherwise use a four-byte encoding
	e = (uint8_t) (rune & 0x3F);
	e |= 0x80;
	rune >>= 6;
	d = (uint8_t) (rune & 0x3F);
	d |= 0x80;
	rune >>= 6;
	c = (uint8_t) (rune & 0x3F);
	c |= 0x80;
	rune >>= 6;
	b = (uint8_t) (rune & 0x07);
	b |= 0xF0;
	n = 4;

done:
	encoded[0] = b;
	if (n > 1)
		encoded[1] = c;
	if (n > 2)
		encoded[2] = d;
	if (n > 3)
		encoded[3] = e;
	return n;
}

const char *uiprivUTF8DecodeRune(const char *s, size_t nElem, uint32_t *rune)
{
	uint8_t b, c;
	uint8_t lowestAllowed, highestAllowed;
	size_t i, expected;
	int bad;

	b = (uint8_t) (*s);
	if (b < 0x80) {		// ASCII bytes represent themselves
		*rune = b;
		s++;
		return s;
	}
	// 0xC0 and 0xC1 cover 2-byte overlong equivalents
	// 0xF5 to 0xFD cover values > 0x10FFFF
	// 0xFE and 0xFF were never defined (always illegal)
	if (b < 0xC2 || b > 0xF4) {		// invalid
		*rune = badrune;
		s++;
		return s;
	}

	// this determines the range of allowed first continuation bytes
	lowestAllowed = 0x80;
	highestAllowed = 0xBF;
	switch (b) {
	case 0xE0:
		// disallow 3-byte overlong equivalents
		lowestAllowed = 0xA0;
		break;
	case 0xED:
		// disallow surrogate characters
		highestAllowed = 0x9F;
		break;
	case 0xF0:
		// disallow 4-byte overlong equivalents
		lowestAllowed = 0x90;
		break;
	case 0xF4:
		// disallow values > 0x10FFFF
		highestAllowed = 0x8F;
		break;
	}

	// and this determines how many continuation bytes are expected
	expected = 1;
	if (b >= 0xE0)
		expected++;
	if (b >= 0xF0)
		expected++;
	if (nElem != 0) {				// are there enough bytes?
		nElem--;
		if (nElem < expected) {	// nope
			*rune = badrune;
			s++;
			return s;
		}
	}

	// ensure that everything is correct
	// if not, **only** consume the initial byte
	bad = 0;
	for (i = 0; i < expected; i++) {
		c = (uint8_t) (s[1 + i]);
		if (c < lowestAllowed || c > highestAllowed) {
			bad = 1;
			break;
		}
		// the old lowestAllowed and highestAllowed is only for the first continuation byte
		lowestAllowed = 0x80;
		highestAllowed = 0xBF;
	}
	if (bad) {
		*rune = badrune;
		s++;
		return s;
	}

	// now do the topmost bits
	if (b < 0xE0)
		*rune = b & 0x1F;
	else if (b < 0xF0)
		*rune = b & 0x0F;
	else
		*rune = b & 0x07;
	s++;		// we can finally move on

	// now do the continuation bytes
	for (; expected; expected--) {
		c = (uint8_t) (*s);
		s++;
		c &= 0x3F;		// strip continuation bits
		*rune <<= 6;
		*rune |= c;
	}

	return s;
}

// encoded must have at most 2 elements
size_t uiprivUTF16EncodeRune(uint32_t rune, uint16_t *encoded)
{
	uint16_t low, high;

	// not in the valid range for Unicode
	if (rune > 0x10FFFF)
		rune = badrune;
	// surrogate runes cannot be encoded
	if (rune >= 0xD800 && rune < 0xE000)
		rune = badrune;

	if (rune < 0x10000) {
		encoded[0] = (uint16_t) rune;
		return 1;
	}

	rune -= 0x10000;
	low = (uint16_t) (rune & 0x3FF);
	rune >>= 10;
	high = (uint16_t) (rune & 0x3FF);
	encoded[0] = high | 0xD800;
	encoded[1] = low | 0xDC00;
	return 2;
}

// TODO see if this can be cleaned up somehow
const uint16_t *uiprivUTF16DecodeRune(const uint16_t *s, size_t nElem, uint32_t *rune)
{
	uint16_t high, low;

	if (*s < 0xD800 || *s >= 0xE000) {
		// self-representing character
		*rune = *s;
		s++;
		return s;
	}
	if (*s >= 0xDC00) {
		// out-of-order surrogates
		*rune = badrune;
		s++;
		return s;
	}
	if (nElem == 1) {		// not enough elements
		*rune = badrune;
		s++;
		return s;
	}
	high = *s;
	high &= 0x3FF;
	if (s[1] < 0xDC00 || s[1] >= 0xE000) {
		// bad surrogate pair
		*rune = badrune;
		s++;
		return s;
	}
	s++;
	low = *s;
	s++;
	low &= 0x3FF;
	*rune = high;
	*rune <<= 10;
	*rune |= low;
	*rune += 0x10000;
	return s;
}

// TODO find a way to reduce the code in all of these somehow
// TODO find a way to remove u as well
size_t uiprivUTF8RuneCount(const char *s, size_t nElem)
{
	size_t len;
	uint32_t rune;

	if (nElem != 0) {
		const char *t, *u;

		len = 0;
		t = s;
		while (nElem != 0) {
			u = uiprivUTF8DecodeRune(t, nElem, &rune);
			len++;
			nElem -= u - t;
			t = u;
		}
		return len;
	}
	len = 0;
	while (*s) {
		s = uiprivUTF8DecodeRune(s, nElem, &rune);
		len++;
	}
	return len;
}

size_t uiprivUTF8UTF16Count(const char *s, size_t nElem)
{
	size_t len;
	uint32_t rune;
	uint16_t encoded[2];

	if (nElem != 0) {
		const char *t, *u;

		len = 0;
		t = s;
		while (nElem != 0) {
			u = uiprivUTF8DecodeRune(t, nElem, &rune);
			len += uiprivUTF16EncodeRune(rune, encoded);
			nElem -= u - t;
			t = u;
		}
		return len;
	}
	len = 0;
	while (*s) {
		s = uiprivUTF8DecodeRune(s, nElem, &rune);
		len += uiprivUTF16EncodeRune(rune, encoded);
	}
	return len;
}

size_t uiprivUTF16RuneCount(const uint16_t *s, size_t nElem)
{
	size_t len;
	uint32_t rune;

	if (nElem != 0) {
		const uint16_t *t, *u;

		len = 0;
		t = s;
		while (nElem != 0) {
			u = uiprivUTF16DecodeRune(t, nElem, &rune);
			len++;
			nElem -= u - t;
			t = u;
		}
		return len;
	}
	len = 0;
	while (*s) {
		s = uiprivUTF16DecodeRune(s, nElem, &rune);
		len++;
	}
	return len;
}

size_t uiprivUTF16UTF8Count(const uint16_t *s, size_t nElem)
{
	size_t len;
	uint32_t rune;
	char encoded[4];

	if (nElem != 0) {
		const uint16_t *t, *u;

		len = 0;
		t = s;
		while (nElem != 0) {
			u = uiprivUTF16DecodeRune(t, nElem, &rune);
			len += uiprivUTF8EncodeRune(rune, encoded);
			nElem -= u - t;
			t = u;
		}
		return len;
	}
	len = 0;
	while (*s) {
		s = uiprivUTF16DecodeRune(s, nElem, &rune);
		len += uiprivUTF8EncodeRune(rune, encoded);
	}
	return len;
}
