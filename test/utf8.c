// 25 may 2020
#include "test.h"

// Do not put any test cases in this file; they will not be run.

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

// TODO utf8diff
