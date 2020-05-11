// 8 june 2019
#include "test.h"
#include "../common/testhooks.h"

static bool vtableNopInit(uiControl *c, void *implData, void *initData)
{
	return true;
}

static void vtableNopFree(uiControl *c, void *implData)
{
	// do nothing
}

// TODO we'll have to eventually find out for real if memset(0) is sufficient to set pointers to NULL or not; C99 doesn't seem to say
Test(ControlImplDataIsClearedOnNewControl)
{
	char memory[32];
	uiControlVtable vt;
	uint32_t type;
	uiControl *c;
	char *implData;

	vt.Size = sizeof (uiControlVtable);
	vt.Init = vtableNopInit;
	vt.Free = vtableNopFree;
	type = uiRegisterControlType("TestControl", &vt, testOSVtable(), sizeof (memory));
	c = uiNewControl(type, NULL);
	implData = (char *) uiControlImplData(c);
	memset(memory, 0, sizeof (memory));
	if (memcmp(implData, memory, sizeof (memory)) != 0)
		TestErrorf("control impl data memory not properly cleared on creation");
	uiControlFree(c);
}

struct counts {
	unsigned int countInit;
	unsigned int countFree;
};

struct testImplData {
	struct counts *counts;
};

static struct counts failInit;
static void *testControlFailInit = &failInit;

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

static uint32_t testControlType(void)
{
	static uint32_t type = 0;

	if (type == 0)
		type = uiRegisterControlType("TestControl", &vtable, testOSVtable(), sizeof (struct testImplData));
	return type;
}

static uint32_t testControlType2(void)
{
	static uint32_t type = 0;

	if (type == 0)
		type = uiRegisterControlType("TestControl2", &vtable, testOSVtable(), sizeof (struct testImplData));
	return type;
}

Test(ControlMethodsCalled)
{
	uiControl *c;
	struct counts counts;

	memset(&counts, 0, sizeof (struct counts));

	c = uiNewControl(testControlType(), &counts);
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

Test(CheckingNonControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): object passed in not a uiControl");
	uiCheckControlType(uiprivTestHookControlWithInvalidControlMarker(), uiControlType());
	endCheckProgrammerError(ctx);
}

Test(CheckingControlWithAnUnknownTypeIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): unknown uiControl type 0 found in uiControl (this is likely not a real uiControl or some data is corrupt)");
	uiCheckControlType(uiprivTestHookControlWithInvalidType(), testControlType());
	endCheckProgrammerError(ctx);
}

Test(CheckingControlWithAnUnknownTypeIsProgrammerErrorEvenIfCheckingAgainstuiControlType)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): unknown uiControl type 0 found in uiControl (this is likely not a real uiControl or some data is corrupt)");
	uiCheckControlType(uiprivTestHookControlWithInvalidType(), uiControlType());
	endCheckProgrammerError(ctx);
}

Test(CheckingForUnknownControlTypeIsProgrammerError)
{
	uiControl *c;
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): unknown uiControl type 0 requested");
	c = uiNewControl(testControlType(), NULL);
	uiCheckControlType(c, 0);
	uiControlFree(c);
	endCheckProgrammerError(ctx);
}

Test(CheckControlTypeFailsCorrectly)
{
	uiControl *c;
	void *ctx;

	ctx = beginCheckProgrammerError("uiCheckControlType(): wrong uiControl type passed: got TestControl, want TestControl2");
	c = uiNewControl(testControlType(), NULL);
	uiCheckControlType(c, testControlType2());
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

	ctx = beginCheckProgrammerError("uiNewControl(): unknown uiControl type 0 requested");
	uiNewControl(0, NULL);
	endCheckProgrammerError(ctx);
}

Test(NewControlWithInvalidInitDataIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiNewControl(): invalid init data for TestControl");
	uiNewControl(testControlType(), testControlFailInit);
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

	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);

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

Test(RemovingParentFromInitiallyParentlessControlIsProgrammerError)
{
	uiControl *c;
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with no parent to have no parent");
	c = uiNewControl(testControlType(), NULL);
	uiControlSetParent(c, NULL);
	uiControlFree(c);
	endCheckProgrammerError(ctx);
}

Test(RemovingParentFromExplicitlyParentlessControlIsProgrammerError)
{
	uiControl *c, *d;
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with no parent to have no parent");
	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);
	uiControlSetParent(c, d);
	uiControlSetParent(c, NULL);
	uiControlSetParent(c, NULL);
	uiControlFree(c);
	uiControlFree(d);
	endCheckProgrammerError(ctx);
}

Test(ReparentingAlreadyParentedControlToDifferentParentIsProgrammerError)
{
	uiControl *c, *d, *e;
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with a parent to have another parent");

	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);
	e = uiNewControl(testControlType(), NULL);

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

Test(ReparentingAlreadyParentedControlToSameParentIsProgrammerError)
{
	uiControl *c, *d;
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with a parent to have another parent");

	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);

	// this should fail
	uiControlSetParent(c, d);
	uiControlSetParent(c, d);

	// this should not (cleanup)
	uiControlSetParent(c, NULL);
	// TODO make sure all cleanups are in reverse order
	uiControlFree(d);
	uiControlFree(c);

	endCheckProgrammerError(ctx);
}

Test(GettingImplDataOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlImplData(): invalid null pointer for uiControl");
	uiControlImplData(NULL);
	endCheckProgrammerError(ctx);
}
