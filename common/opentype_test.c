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
	uint32_t value;

	otf = uiNewOpenTypeFeatures();
	testingTDefer(t, freeOpenType, otf);
	uiOpenTypeFeaturesAdd(otf, 'a', 'b', 'c', 'd', 12345);
	if (!uiOpenTypeFeaturesGet(otf, 'a', 'b', 'c', 'd', &value))
		testingTFatalf(t, "uiOpenTypeFeaturesGet() failed to get feature we added");
	if (value != 12345)
		testingTFatalf(t, "feature abcd: got %" PRIu32 ", want 12345", value);
}

int main(void)
{
	return testingMain();
}
