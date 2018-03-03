// 27 february 2018
#ifndef TODO_TEST
#error TODO this is where libui itself goes
#endif
#include <inttypes.h>
#include "testing.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef struct uiOpenTypeFeatures uiOpenTypeFeatures;
typedef int uiForEach;
enum { uiForEachContinue, uiForEachStop };
typedef uiForEach (*uiOpenTypeFeaturesForEachFunc)(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data);
#define uiprivNew(x) ((x *) malloc(sizeof (x)))
#define uiprivAlloc(x,y) malloc(x)
#define uiprivRealloc(x,y,z) realloc(x,y)
#define uiprivFree free
#include "opentype.c"

static void freeOpenType(void *otf)
{
	uiFreeOpenTypeFeatures((uiOpenTypeFeatures *) otf);
}

testingTest(OpenTypeFeaturesAddGet)
{
	uiOpenTypeFeatures *otf;
	int got;
	uint32_t value;

	otf = uiNewOpenTypeFeatures();
	testingTDefer(t, freeOpenType, otf);
	uiOpenTypeFeaturesAdd(otf, 'a', 'b', 'c', 'd', 12345);
	got = uiOpenTypeFeaturesGet(otf, 'a', 'b', 'c', 'd', &value);

	if (!got)
		testingTErrorf(t, "uiOpenTypeFeaturesGet() failed to get feature we added");
	else if (value != 12345)
		testingTErrorf(t, "feature abcd: got %" PRIu32 ", want 12345", value);
}

testingTest(OpenTypeFeaturesRemove)
{
	uiOpenTypeFeatures *otf;
	uint32_t value;

	otf = uiNewOpenTypeFeatures();
	testingTDefer(t, freeOpenType, otf);
	uiOpenTypeFeaturesAdd(otf, 'a', 'b', 'c', 'd', 12345);
	uiOpenTypeFeaturesRemove(otf, 'a', 'b', 'c', 'd');

	if (uiOpenTypeFeaturesGet(otf, 'a', 'b', 'c', 'd', &value))
		testingTErrorf(t, "uiOpenTypeFeaturesGet() succeeded in getting deleted feature; value %" PRIu32, value);
}

testingTest(OpenTypeFeaturesCloneAdd)
{
	uiOpenTypeFeatures *otf, *otf2;
	uint32_t value;

	otf = uiNewOpenTypeFeatures();
	testingTDefer(t, freeOpenType, otf);
	uiOpenTypeFeaturesAdd(otf, 'a', 'b', 'c', 'd', 12345);
	otf2 = uiOpenTypeFeaturesClone(otf);
	testingTDefer(t, freeOpenType, otf2);
	uiOpenTypeFeaturesAdd(otf2, 'q', 'w', 'e', 'r', 56789);

	if (uiOpenTypeFeaturesGet(otf, 'q', 'w', 'e', 'r', &value))
		testingTErrorf(t, "uiOpenTypeFeaturesGet() on original succeeded in getting feature added to clone; value %" PRIu32, value);
}

testingTest(OpenTypeFeaturesCloneModify)
{
	uiOpenTypeFeatures *otf, *otf2;
	uint32_t value;

	otf = uiNewOpenTypeFeatures();
	testingTDefer(t, freeOpenType, otf);
	uiOpenTypeFeaturesAdd(otf, 'a', 'b', 'c', 'd', 12345);
	otf2 = uiOpenTypeFeaturesClone(otf);
	testingTDefer(t, freeOpenType, otf2);
	uiOpenTypeFeaturesAdd(otf2, 'a', 'b', 'c', 'd', 56789);

	uiOpenTypeFeaturesGet(otf, 'a', 'b', 'c', 'd', &value);
	if (value != 12345)
		testingTErrorf(t, "uiOpenTypeFeaturesGet() on original: got %" PRIu32 ", want 12345", value);
	uiOpenTypeFeaturesGet(otf2, 'a', 'b', 'c', 'd', &value);
	if (value != 56789)
		testingTErrorf(t, "uiOpenTypeFeaturesGet() on clone: got %" PRIu32 ", want 56789", value);
}

testingTest(OpenTypeFeaturesCloneRemove)
{
	uiOpenTypeFeatures *otf, *otf2;
	uint32_t value;

	otf = uiNewOpenTypeFeatures();
	testingTDefer(t, freeOpenType, otf);
	uiOpenTypeFeaturesAdd(otf, 'a', 'b', 'c', 'd', 12345);
	otf2 = uiOpenTypeFeaturesClone(otf);
	testingTDefer(t, freeOpenType, otf2);
	uiOpenTypeFeaturesRemove(otf2, 'a', 'b', 'c', 'd');

	if (uiOpenTypeFeaturesGet(otf2, 'a', 'b', 'c', 'd', &value))
		testingTErrorf(t, "uiOpenTypeFeaturesGet() on clone succeeded in getting feature removed from clone; value %" PRIu32, value);
	if (!uiOpenTypeFeaturesGet(otf, 'a', 'b', 'c', 'd', &value))
		testingTErrorf(t, "uiOpenTypeFeaturesGet() on original failed to get feature removed from clone");
}

int main(void)
{
	return testingMain();
}
