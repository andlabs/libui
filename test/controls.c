// 8 june 2019
#include "test.h"

struct counts {
	unsigned int countInit;
	unsigned int countFree;
};

struct testImplData {
	struct counts *counts;
};

static struct counts failInit;
static void *testControlFailInit = &failInit;

// TODO document that impl data is zero-initialized before this is called
// TODO we'll also have to eventually deal with the fact that NULL is not required to be 0... or at least confirm that
static bool testVtableInit(uiControl *c, void *implData, void *initData)
{
	struct testImplData *d = (struct testImplData *) implData;
	struct counts *counts = (struct counts *) initData;

	if (initData == testControlFailInit)
		return false;
	if (initData == NULL)
		return true;
	if (d->counts == NULL)
		d->counts = counts;
	d->counts->countInit++;
	if (d->counts->countInit > 2)
		d->counts->countInit = 2;
	return true;
}

static void testVtableFree(uiControl *c, void *implData)
{
	struct testImplData *d = (struct testImplData *) implData;

	if (d->counts != NULL) {
		d->counts->countFree++;
		if (d->counts->countFree > 2)
			d->counts->countFree = 2;
	}
}

static const uiControlVtable vtable = {
	.Size = sizeof (uiControlVtable),
	.Init = testVtableInit,
	.Free = testVtableFree,
};

// TODO explicitly make/document 0 as always invalid
static uint32_t testControlType = 0;

Test(ControlMethodsCalled)
{
	uiControl *c;
	struct counts counts;

	testControlType = uiRegisterControlType("TestControl2", &vtable, testOSVtable(), sizeof (struct testImplData));
	memset(&counts, 0, sizeof (struct counts));

	c = uiNewControl(testControlType, &counts);
	switch (counts.countInit) {
	case 0:
		TestErrorf("Init() was not called");
		break;
	case 1:
		// do nothing; this is the expected case
		break;
	default:
		TestErrorf("Init() called more than once");
	}
	if (counts.countFree != 0)
		TestErrorf("Free() called unexpectedly by uiNewControl()");

	uiControlFree(c);
	switch (counts.countFree) {
	case 0:
		TestErrorf("Free() was not called");
		break;
	case 1:
		// do nothing; this is the expected case
		break;
	default:
		TestErrorf("Free() called more than once");
	}
	if (counts.countInit != 1)
		TestErrorf("Init() called unexpectedly by uiControlFree()");
}
