// 8 june 2019
#include "test.h"

struct testInitData {
	unsigned long *freeCount;
};

struct testImplData {
	bool initCalled;
	unsigned long *freeCount;
};

static struct testInitData failInit;
void *testControlFailInit = &failInit;

// TODO document that impl data is zero-initialized before this is called
static bool testVtableInit(uiControl *c, void *implData, void *initData)
{
	struct testImplData *d = (struct testImplData *) implData;
	struct testInitData *tid = (struct testInitData *) initData;

	d->initCalled = true;
	if (tid == testControlFailInit)
		return false;
	if (tid == NULL)
		return true;
	d->freeCount = tid->freeCount;
	return true;
}

static void testVtableFree(uiControl *c, void *implData)
{
	struct testImplData *d = (struct testImplData *) implData;

	if (d->freeCount != NULL)
		(*(d->freeCount))++;
}

static const uiControlVtable vtable = {
	.Size = sizeof (uiControlVtable),
	.Init = testVtableInit,
	.Free = testVtableFree,
};

const uiControlVtable *testVtable(void)
{
	return &vtable;
}

size_t testImplDataSize(void)
{
	return sizeof (struct testImplData);
}

// TODO explicitly make/document 0 as always invalid
uint32_t testControlType = 0;
uint32_t testControlType2 = 0;

testingTest(ControlMethodsCalled)
{
	uiControl *c;
	struct testImplData *d;
	struct testInitData tid;
	unsigned long freeCount = 0;

	memset(&tid, 0, sizeof (struct testInitData));
	tid.freeCount = &freeCount;
	c = uiNewControl(testControlType, &tid);
	d = (struct testImplData *) uiControlImplData(c);
	if (d == NULL)
		testingTErrorf(t, "uiControlImplData() returned NULL; should not have");
	else if (!d->initCalled)
		testingTErrorf(t, "uiNewControl() did not call Init(); should have");
	// TODO add event handler
	uiControlFree(c);
	if (freeCount != 1)
		testingTErrorf(t, "uiControlFree() wrong Free() call count:" diff("%lu"),
			freeCount, 1UL);
}

// TODO test freeing a parent frees the child
