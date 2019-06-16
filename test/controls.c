// 8 june 2019
#include "test.h"

struct testImplData {
	bool initCalled;
	bool *freeCalled;
	bool testMethodCalled;
};

static bool testVtableInit(uiControl *c, void *implData, void *initData)
{
	return true;
}

static void testVtableFree(uiControl *c, void *implData)
{
	// do nothing
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
