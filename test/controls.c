// 8 june 2019
#include "test.h"

struct testInitData {
	bool *freeCalled;
};

struct testImplData {
	bool initCalled;
	bool *freeCalled;
};

static struct testInitData failInit;
void *testControlFailInit = &failInit;

static bool testVtableInit(uiControl *c, void *implData, void *initData)
{
	struct testImplData *d = (struct testInitData *) implData;
	struct testInitData *tid = (struct testInitData *) initData;

	d->initCalled = true;
	if (tid == testControlFailInit)
		return false;
	if (tid == NULL)
		return true;
	d->freeCalled = tid->freeCalled;
	return true;
}

static void testVtableFree(uiControl *c, void *implData)
{
	struct testImplData *d = (struct testInitData *) implData;

	if (d->freeCalled != NULL)
		*(d->freeCalled) = true;
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
	bool freeCalled = false;

	memset(&tid, 0, sizeof (struct testInitData));
	tid.freeCalled = &freeCalled;
	c = uiNewControl(testControlType, &tid);
	d = (struct testImplData *) uiControlImplData(c);
	if (d == NULL)
		testingTErrorf(t, "uiControlImplData() returned NULL; should not have");
	else if (!d->initCalled)
		testingTErrorf(t, "uiNewControl() did not call Init(); should have");
	// TODO add event handler
	uiControlFree(c);
	if (!freeCalled)
		testingTErrorf(t, "uiControlFree() did not call Free(); should have");
}

// TODO test freeing a parent frees the child
