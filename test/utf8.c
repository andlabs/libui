// 25 may 2020
#include "test.h"

// Do not put any test cases in this file; they will not be run.

/* TODO address all this
C99 notes on the safety of storing UTF-8 data directly in char:
- §5.2.4.2.1¶1 says char, signed char, and unsigned char must be no fewer than 8 bits wide
- §6.2.5¶15 states that char must either be signed char or unsigned char (or some equivalent)
	- footnote 35 says that CHAR_MIN can be used to detect which one was chosen, which can help in some of the situations below
	- §6.3.1.1¶1 ranks the three equally for conversions
- either:
	- TODO verify that integer conversions from signed char/unsgined char to char do simple bit reinterpretations, or
		- §6.3.1.3¶2 suggests that signed->unsigned conversions are legal and simply do (abstract mathematical value of signed value)+((UTYPE_MAX+1)*necessary amount to make the result unsigned)
		- §6.3.1.3¶3 suggests that unsigned->signed conversions where the value is greater than the positive maximum are totally implementation-defined
	- TODO verify that octal and hex escape sequences always result in a constant whose bit value is the same as what's specified in the esacpe sequence, regardless of the execution character set
		- neither §6.4.4.4 nor §6.4.5 say anything about this
			- §6.4.4.4¶5 and ¶6 both use the ambiguous phrase "the value of the desired character", which does not state whether this is before or after conversion to the execution character set, or if it means the actual bit value of the character constant
		- §6.4.4.4¶13 **implies** that this is true, because it shows \xFF leading to either -1 or 255 on a 2's complement 8-bit-byte system without making any statement about the execution character set
		- §6.4.4.4¶9 states that "the value" of an octal or hex escape sequence must be representable by unsigned char, but this doesn't state whether that means the unprocessed integer that the sequence denotes, the resultant character value before or after conversion to the execution character set, or the resultant constant's bit value
		- howerver, §6.4.5¶7 implies that octal and hex escape sequences in *strings* **DO** undergo execution character set conversion
		- only relevant Google results I can find are for gcc's C preprocessor which says it does not do that conversion
- §5.2.1¶2 requires that the null character have all bits set to 0 in the execution character set
- §5.2.1.2¶1 requires that the null character not appear in the middle of a multi-byte character sequence regardless of the execution character set
- TODO verify that strlen() and strcmp() indeed check one unsigned char unit at a time, looking for that null character (this requires all of the above to be true in order to make strcmp() useful to us)
	- §7.21.4¶1 says that strcmp() operates on units of unsigned chars
	- §7.21.6.3 does not state at all what strlen() considers to be a character; we may need to roll our own...
- OPTIONAL TODO verify that string literals containing only octal and hex escape sequences undergo no execution character set translation if character literals don't

https://stackoverflow.com/q/435147/3408572 says that the Unicode standard claims to suggest using char for UTF-8; it links to http://www.unicode.org/versions/Unicode5.0.0/ch05.pdf#G23927 which talks about how ISO/IEC Technical Report 19769 seems to suggest using it for UTF-8, but the closest freely available draft, http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1040.pdf, only talks about some implementations using char for UTF-8 and that the TR is really all about char16_t and char32_t

For C11, u8"..." is explicitly documented in §6.4.5¶6 as storing its UTF-8-encoded bytes in chars. This means we can't just require the use of unsigned char, since that'll make it impossible for C11 users to be able to use u8 literals.
Also, C11 §6.4.4.4 does not fix the problems mentioned above :| nor does it say anything about strlen()
*/

// TODO specify these hex constants in a signedness-safe way
const char testUTF8Empty[] = { 0 };
const char testUTF8ASCIIOnly[] = { 0x74, 0x65, 0x73, 0x74, 0 };
const char testUTF8WithTwoByte[] = { 0x74, 0xC3, 0xA9, 0x73, 0x74, 0 };
const char testUTF8WithThreeByte[] = { 0x74, 0xE2, 0x93, 0x94, 0x73, 0x74, 0 };
const char testUTF8WithFourByte[] = { 0x74, 0xF0, 0x9D, 0x90, 0x9E, 0x73, 0x74, 0 };
// TODO document that libui does not do any sort of Unicode normalization; TODO test that (the below is adequate for canonicalization tests vs WithTwoByte above)
const char testUTF8Combined[] = { 0x74, 0x65, 0xCC, 0x81, 0x73, 0x74, 0 };
const char testUTF8InvalidInput[] = { 0x74, 0xC3, 0x29, 0x73, 0x74, 0 };
const char testUTF8InvalidOutput[] = { 0x74, 0xEF, 0xBF, 0xBD, 0x29, 0x73, 0x74, 0 };

// TODO figure out if strcmp() is adequate for this
bool utf8equal(const char *s, const char *t)
{
	for (;;) {
		if (*s == 0 && *t == 0)
			return true;
		if (*s == 0 || *t == 0)
			return false;
		if (*s != *t)
			return false;
		s++;
		t++;
	}
}

static void utf8hexdump(char buf[64], const char *s)
{
	int i;
	uint8_t x;

	for (i = 0; i < 60; i += 3) {
		x = (uint8_t) (*s);
		s++;
		buf[i + 0] = "0123456789ABCDEF"[(x & 0xF0) >> 4];
		buf[i + 1] = "0123456789ABCDEF"[x & 0x0F];
		if (x == 0) {
			buf[i + 2] = '\0';
			break;
		}
		buf[i + 2] = ' ';
	}
	if (i >= 60) {
		buf[60] = '.';
		buf[61] = '.';
		buf[62] = '.';
		buf[63] = '\0';
	}
}

void utf8diffErrorFull(const char *file, long line, const char *msg, const char *got, const char *want)
{
	char a[64], b[64];

	utf8hexdump(a, got);
	utf8hexdump(b, want);
	TestErrorfFull(file, line, "%s:" diff("%s"), msg, a, b);
}
