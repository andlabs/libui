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

const uiControlVtable dummyVtable = {
	Size:		sizeof (uiControlVtable),
	Init:		testVtableInit,
	Free:		testVtableFree,
};
