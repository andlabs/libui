// 8 june 2019
#include "test.h"

// TODO replace hardcoded control types of 5 with a constant from the test hook system

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

Test(NullControlTypeNameIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): invalid null pointer for name");
	uiRegisterControlType(NULL, NULL, NULL, 0);
	endCheckProgrammerError(ctx);
}

Test(NullControlVtableIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): invalid null pointer for uiControlVtable");
	uiRegisterControlType("name", NULL, NULL, 0);
	endCheckProgrammerError(ctx);
}

Test(WrongControlVtableSizeIsProgrammerError)
{
	uiControlVtable vtable;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): wrong size 1 for uiControlVtable");
	memset(&vtable, 0, sizeof (uiControlVtable));
	vtable.Size = 1;
	uiRegisterControlType("name", &vtable, NULL, 0);
	endCheckProgrammerError(ctx);
}

Test(ControlVtableWithMIssingInitMethodIsProgrammerError)
{
	uiControlVtable vt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlVtable method Init() missing for uiControl type name");
	vt = vtable;
	vt.Init = NULL;
	uiRegisterControlType("name", &vt, NULL, 0);
	endCheckProgrammerError(ctx);
}

Test(ControlVtableWithMIssingFreeMethodIsProgrammerError)
{
	uiControlVtable vt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlVtable method Free() missing for uiControl type name");
	vt = vtable;
	vt.Free = NULL;
	uiRegisterControlType("name", &vt, NULL, 0);
	endCheckProgrammerError(ctx);
}

Test(NullControlOSVtableIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): invalid null pointer for uiControlOSVtable");
	uiRegisterControlType("name", &vtable, NULL, 0);
	endCheckProgrammerError(ctx);
}

Test(CheckingNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): invalid null pointer for uiControl");
	uiCheckControlType(NULL, uiControlType());
	endCheckProgrammerError(ctx);
}

// TODO should this test be the same as the following one instead?
Test(CheckingNonControlIsProgrammerError)
{
	// TODO make sure this is fine; if not, just use the following check instead
	static char buf[] = "this is not a uiControl but is big enough to at the very least not cause a problem with UB hopefully";
	void *ctx;

	ctx = beginCheckProgrammerError("TODO");
	uiCheckControlType(buf, uiControlType());
	endCheckProgrammerError(ctx);
}

Test(CheckingControlWithoutControlMarkerIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): object passed in not a uiControl");
	uiCheckControlType(uiprivTestHookControlWithInvalidControlMarker(), uiControlType());
	endCheckProgrammerError(ctx);
}

Test(CheckingControlWithAnUnknownTypeIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): unknown uiControl type 5 found in uiControl (this is likely not a real uiControl or some data is corrupt)");
	uiCheckControlType(uiprivTestHookControlWithInvalidType(), testControlType);
	endCheckProgrammerError(ctx);
}

Test(CheckingControlWithAnUnknownTypeIsProgrammerErrorEvenIfCheckingAgainstuiControlType)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): unknown uiControl type 5 found in uiControl (this is likely not a real uiControl or some data is corrupt)");
	uiCheckControlType(uiprivTestHookControlWithInvalidType(), uiControlType());
	endCheckProgrammerError(ctx);
}

Test(CheckingForUnknownControlTypeIsProgrammerError)
{
	uiControl *c;
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): unknown uiControl type 5 requested");
	c = uiNewControl(testControlType, NULL);
	uiCheckControlType(c, 5);
	uiControlFree(c);
	endCheckProgrammerError(ctx);
}

Test(CheckControlTypeFailsCorrectly)
{
	uiControl *c;
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): wrong uiControl type passed: got TestControl, want TestControl2");
	c = uiNewControl(testControlType, NULL);
	uiCheckControlType(c, testControlType2);
	uiControlFree(c);
	endCheckProgrammerError(ctx);
}

Test(NewControlOfTypeControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiNewControl(): uiControlType() passed in when specific control type needed");
	uiNewControl(uiControlType(), NULL);
	endCheckProgrammerError(ctx);
}

Test(NewControlOfUnknownTypeIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiNewControl(): unknown uiControl type 5 requested");
	uiNewControl(5, NULL);
	endCheckProgrammerError(ctx);
}

Test(NewControlWithInvalidInitDataIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiNewControl(): invalid init data for TestControl");
	uiNewControl(testControlType, testControlFailInit);
	endCheckProgrammerError(ctx);
}

Test(FreeingNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlFree(): invalid null pointer for uiControl");
	uiControlFree(NULL);
	endCheckProgrammerError(ctx);
}

// TODO go back through all these tests and make the programmer error check as finely scoped as necessary
Test(FreeingParentedControlIsProgrammerError)
{
	uiControl *c, *d;
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlFree(): cannot be called on a control with has a parent");

	c = uiNewControl(testControlType, NULL);
	d = uiNewControl(testControlType, NULL);

	// this should fail
	uiControlSetParent(c, d);
	uiControlFree(c);

	// this should not fail; it's normal cleanup
	uiControlSetParent(c, NULL);
	uiControlFree(d);
	uiControlFree(c);

	endCheckProgrammerError(ctx);
}

Test(SetParentWithNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlSetParent(): invalid null pointer for uiControl");
	uiControlSetParent(NULL, NULL);
	endCheckProgrammerError(ctx);
}

// TODO copy this test but first setting and then removing the parent first, instead of just testing the initial state
Test(RemovingParentFromParentlessControlIsProgrammerError)
{
	uiControl *c;
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with no parent to have no parent");
	c = uiNewControl(testControlType, NULL);
	uiControlSetParent(c, NULL);
	uiControlFree(c);
	endCheckProgrammerError(ctx);
}

Test(ReparentingAlreadyParentedControlIsProgrammerError)
{
	uiControl *c, *d, *e;
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with a parent to have another parent");

	c = uiNewControl(testControlType, NULL);
	d = uiNewControl(testControlType, NULL);
	e = uiNewControl(testControlType, NULL);

	// this should fail
	uiControlSetParent(c, d);
	uiControlSetParent(c, e);

	// this should not (cleanup)
	uiControlSetParent(c, NULL);
	uiControlFree(e);
	uiControlFree(d);
	uiControlFree(c);

	endCheckProgrammerError(ctx);
}

// TODO define and then test the above but for the same parent

Test(GettingImplDataOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlImplData(): invalid null pointer for uiControl");
	uiControlImplData(NULL);
	endCheckProgrammerError(ctx);
}
