// 8 june 2019
#include "test.h"

static bool vtableNopInit(uiControl *c, void *implData, void *initData)
{
	return true;
}

static void vtableNopFree(uiControl *c, void *implData)
{
	// do nothing
}

static void vtableNopParentChanging(uiControl *c, void *implData, uiControl *oldParent)
{
	// do nothing
}

static void vtableNopParentChanged(uiControl *c, void *implData, uiControl *newParent)
{
	// do nothing
}

void testControlLoadNopVtable(uiControlVtable *vtable)
{
	memset(vtable, 0, sizeof (uiControlVtable));
	vtable->Size = sizeof (uiControlVtable);
	vtable->Init = vtableNopInit;
	vtable->Free = vtableNopFree;
	vtable->ParentChanging = vtableNopParentChanging;
	vtable->ParentChanged = vtableNopParentChanged;
}

// TODO we'll have to eventually find out for real if memset(0) is sufficient to set pointers to NULL or not; C99 doesn't seem to say
Test(ControlImplDataIsClearedOnNewControl)
{
	char memory[32];
	uiControlVtable vt;
	uint32_t type;
	uiControl *c;
	char *implData;

	testControlLoadNopVtable(&vt);
	type = uiRegisterControlType("TestControl", &vt, testOSVtable(), sizeof (memory));
	c = uiNewControl(type, NULL);
	implData = (char *) uiControlImplData(c);
	memset(memory, 0, sizeof (memory));
	if (memcmp(implData, memory, sizeof (memory)) != 0)
		TestErrorf("control impl data memory not properly cleared on creation");
	uiControlFree(c);
}

Test(ZeroSizeImplDataIsNULL)
{
	uiControlVtable vt;
	uint32_t type;
	uiControl *c;

	testControlLoadNopVtable(&vt);
	type = uiRegisterControlType("TestControl", &vt, testOSVtable(), 0);
	c = uiNewControl(type, NULL);
	if (uiControlImplData(c) != NULL)
		TestErrorf("control impl data is non-NULL despite being of size 0");
	uiControlFree(c);
}

struct counts {
	unsigned int countInit;
	unsigned int countFree;
	unsigned int countParentChanging;
	unsigned int countParentChanged;
	uiControl *oldParent;
	uiControl *newParent;
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

static void testVtableParentChanging(uiControl *c, void *implData, uiControl *oldParent)
{
	struct testImplData *d = (struct testImplData *) implData;

	if (d->counts != NULL) {
		d->counts->oldParent = oldParent;
		d->counts->countParentChanging++;
		if (d->counts->countParentChanging > 3)
			d->counts->countParentChanging = 3;
	}
}

static void testVtableParentChanged(uiControl *c, void *implData, uiControl *newParent)
{
	struct testImplData *d = (struct testImplData *) implData;

	if (d->counts != NULL) {
		d->counts->newParent = newParent;
		d->counts->countParentChanged++;
		if (d->counts->countParentChanged > 3)
			d->counts->countParentChanged = 3;
	}
}


static const uiControlVtable vtable = {
	.Size = sizeof (uiControlVtable),
	.Init = testVtableInit,
	.Free = testVtableFree,
	.ParentChanging = testVtableParentChanging,
	.ParentChanged = testVtableParentChanged,
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

// TODO do this but for the OS-specific methods
Test(ControlMethodsCalled)
{
	uiControl *c, *d;
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
	// yes, the casts to void * are necessary, because the "equivalence" of data pointers to void * is really just the compiler doing conversions for you and this does not (and cannot) extend to the parameter lists of varargs functions (https://stackoverflow.com/questions/34723062, https://stackoverflow.com/questions/9053658)
	if (counts.countParentChanging != 0)
		TestErrorf("ParentChanging() called unexpectedly by uiNewControl(); most recent oldParent = %p", (void *) (counts.oldParent));
	if (counts.countParentChanged != 0)
		TestErrorf("ParentChanged() called unexpectedly by uiNewControl(); most recent newParent = %p", (void *) (counts.newParent));

	d = uiNewControl(testControlType(), NULL);

	uiControlSetParent(c, d);
	switch (counts.countParentChanging) {
	case 0:
		TestErrorf("ParentChanging() was not called by SetParent(non-NULL)");
		break;
	case 1:
		// do nothing; this is the expected case
		break;
	default:
		TestErrorf("ParentChanging() called more than once by SetParent(non-NULL)");
	}
	if (counts.oldParent != NULL)
		TestErrorf("ParentChanging() called with wrong oldParent by SetParent(non-NULL) (if called more than once, this is the most recent call):" diff("%p"),
			(void *) (counts.oldParent), (void *) NULL);
	switch (counts.countParentChanged) {
	case 0:
		TestErrorf("ParentChanged() was not called by SetParent(non-NULL)");
		break;
	case 1:
		// do nothing; this is the expected case
		break;
	default:
		TestErrorf("ParentChanged() called more than once by SetParent(non-NULL)");
	}
	if (counts.newParent != d)
		TestErrorf("ParentChanged() called with wrong newParent by SetParent(non-NULL) (if called more than once, this is the most recent call):" diff("%p"),
			(void *) (counts.newParent), (void *) d);
	if (counts.countInit != 1)
		TestErrorf("Init() called unexpectedly by uiControlSetParent(non-NULL)");
	if (counts.countFree != 0)
		TestErrorf("Free() called unexpectedly by uiControlSetParent(non-NULL)");

	uiControlSetParent(c, NULL);
	switch (counts.countParentChanging) {
	case 0:
	case 1:
		TestErrorf("ParentChanging() was not called by SetParent(NULL)");
		break;
	case 2:
		// do nothing; this is the expected case
		break;
	default:
		TestErrorf("ParentChanging() called more than once by SetParent(NULL)");
	}
	if (counts.oldParent != d)
		TestErrorf("ParentChanging() called with wrong oldParent by SetParent(NULL) (if called more than once, this is the most recent call):" diff("%p"),
			(void *) (counts.oldParent), (void *) d);
	switch (counts.countParentChanged) {
	case 0:
	case 1:
		TestErrorf("ParentChanged() was not called by SetParent(NULL)");
		break;
	case 2:
		// do nothing; this is the expected case
		break;
	default:
		TestErrorf("ParentChanged() called more than once by SetParent(NULL)");
	}
	if (counts.newParent != NULL)
		TestErrorf("ParentChanged() called with wrong newParent by SetParent(NULL) (if called more than once, this is the most recent call):" diff("%p"),
			(void *) (counts.newParent), (void *) NULL);
	if (counts.countInit != 1)
		TestErrorf("Init() called unexpectedly by uiControlSetParent(NULL)");
	if (counts.countFree != 0)
		TestErrorf("Free() called unexpectedly by uiControlSetParent(NULL)");

	uiControlFree(d);

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
	if (counts.countParentChanging != 2)
		TestErrorf("ParentChanging() called unexpectedly by uiNewControl(); most recent oldParent = %p", (void *) (counts.oldParent));
	if (counts.countParentChanged != 2)
		TestErrorf("ParentChanged() called unexpectedly by uiNewControl(); most recent newParent = %p", (void *) (counts.newParent));
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

Test(ControlVtableWithMissingInitMethodIsProgrammerError)
{
	uiControlVtable vt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlVtable method Init() missing for uiControl type name");
	vt = vtable;
	vt.Init = NULL;
	uiRegisterControlType("name", &vt, NULL, 0);
	endCheckProgrammerError(ctx);
}

Test(ControlVtableWithMissingFreeMethodIsProgrammerError)
{
	uiControlVtable vt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlVtable method Free() missing for uiControl type name");
	vt = vtable;
	vt.Free = NULL;
	uiRegisterControlType("name", &vt, NULL, 0);
	endCheckProgrammerError(ctx);
}

Test(ControlVtableWithMissingParentChangingMethodIsProgrammerError)
{
	uiControlVtable vt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlVtable method ParentChanging() missing for uiControl type name");
	vt = vtable;
	vt.ParentChanging = NULL;
	uiRegisterControlType("name", &vt, NULL, 0);
	endCheckProgrammerError(ctx);
}

Test(ControlVtableWithMissingParentChangedMethodIsProgrammerError)
{
	uiControlVtable vt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlVtable method ParentChanged() missing for uiControl type name");
	vt = vtable;
	vt.ParentChanged = NULL;
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
	uint32_t ctrlType;
	void *ctx;

	ctrlType = uiControlType();
	ctx = beginCheckProgrammerError("uiCheckControlType(): invalid null pointer for uiControl");
	uiCheckControlType(NULL, ctrlType);
	endCheckProgrammerError(ctx);
}

Test(CheckingNonControlIsProgrammerError)
{
	uiControl *c;
	uint32_t ctrlType;
	void *ctx;

	c = uiprivTestHookControlWithInvalidControlMarker();
	ctrlType = uiControlType();
	ctx = beginCheckProgrammerError("uiCheckControlType(): object passed in not a uiControl");
	uiCheckControlType(c, ctrlType);
	endCheckProgrammerError(ctx);
}

Test(CheckingControlWithAnUnknownTypeIsProgrammerError)
{
	uiControl *c;
	uint32_t ctrlType;
	void *ctx;

	c = uiprivTestHookControlWithInvalidType();
	ctrlType = testControlType();
	ctx = beginCheckProgrammerError("uiCheckControlType(): unknown uiControl type 0 found in uiControl (this is likely not a real uiControl or some data is corrupt)");
	uiCheckControlType(c, ctrlType);
	endCheckProgrammerError(ctx);
}

Test(CheckingControlWithAnUnknownTypeIsProgrammerErrorEvenIfCheckingAgainstuiControlType)
{
	uiControl *c;
	uint32_t ctrlType;
	void *ctx;

	c = uiprivTestHookControlWithInvalidType();
	ctrlType = uiControlType();
	ctx = beginCheckProgrammerError("uiCheckControlType(): unknown uiControl type 0 found in uiControl (this is likely not a real uiControl or some data is corrupt)");
	uiCheckControlType(c, ctrlType);
	endCheckProgrammerError(ctx);
}

Test(CheckingForUnknownControlTypeIsProgrammerError)
{
	uiControl *c;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	ctx = beginCheckProgrammerError("uiCheckControlType(): unknown uiControl type 0 requested");
	uiCheckControlType(c, 0);
	endCheckProgrammerError(ctx);
	uiControlFree(c);
}

Test(CheckControlTypeFailsCorrectly)
{
	uiControl *c;
	uint32_t ctrlType;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	ctrlType = testControlType2();
	ctx = beginCheckProgrammerError("uiCheckControlType(): wrong uiControl type passed: got TestControl, want TestControl2");
	uiCheckControlType(c, ctrlType);
	endCheckProgrammerError(ctx);
	uiControlFree(c);
}

Test(NewControlOfTypeControlIsProgrammerError)
{
	uint32_t ctrlType;
	void *ctx;

	ctrlType = uiControlType();
	ctx = beginCheckProgrammerError("uiNewControl(): uiControlType() passed in when specific control type needed");
	uiNewControl(ctrlType, NULL);
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
	uint32_t ctrlType;
	void *ctx;

	ctrlType = testControlType();
	ctx = beginCheckProgrammerError("uiNewControl(): invalid init data for TestControl");
	uiNewControl(ctrlType, testControlFailInit);
	endCheckProgrammerError(ctx);
}

Test(FreeingNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlFree(): invalid null pointer for uiControl");
	uiControlFree(NULL);
	endCheckProgrammerError(ctx);
}

Test(FreeingParentedControlIsProgrammerError)
{
	uiControl *c, *d;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);

	uiControlSetParent(c, d);
	ctx = beginCheckProgrammerError("uiControlFree(): cannot be called on a control with has a parent");
	uiControlFree(c);
	endCheckProgrammerError(ctx);

	// cleanup
	uiControlSetParent(c, NULL);
	uiControlFree(d);
	uiControlFree(c);
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

	c = uiNewControl(testControlType(), NULL);
	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with no parent to have no parent");
	uiControlSetParent(c, NULL);
	endCheckProgrammerError(ctx);
	uiControlFree(c);
}

Test(RemovingParentFromExplicitlyParentlessControlIsProgrammerError)
{
	uiControl *c, *d;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);
	uiControlSetParent(c, d);
	uiControlSetParent(c, NULL);
	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with no parent to have no parent");
	uiControlSetParent(c, NULL);
	endCheckProgrammerError(ctx);
	uiControlFree(d);
	uiControlFree(c);
}

Test(ReparentingAlreadyParentedControlToDifferentParentIsProgrammerError)
{
	uiControl *c, *d, *e;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);
	e = uiNewControl(testControlType(), NULL);

	uiControlSetParent(c, d);
	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with a parent to have another parent");
	uiControlSetParent(c, e);
	endCheckProgrammerError(ctx);

	// cleanup
	uiControlSetParent(c, NULL);
	uiControlFree(e);
	uiControlFree(d);
	uiControlFree(c);
}

Test(ReparentingAlreadyParentedControlToSameParentIsProgrammerError)
{
	uiControl *c, *d;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);

	uiControlSetParent(c, d);
	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot set a control with a parent to have another parent");
	uiControlSetParent(c, d);
	endCheckProgrammerError(ctx);

	// cleanup
	uiControlSetParent(c, NULL);
	uiControlFree(d);
	uiControlFree(c);
}

Test(ControlParentCyclesDisallowed_TwoControls)
{
	uiControl *c, *d;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);

	uiControlSetParent(c, d);
	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot create a parent cycle");
	uiControlSetParent(d, c);
	endCheckProgrammerError(ctx);

	// cleanup
	// TODO reformat all the other tests to have clear init, test, and cleanup sections, and also maybe remove these "// cleanup" comments
	uiControlSetParent(c, NULL);
	uiControlFree(d);
	uiControlFree(c);
}

Test(ControlParentCyclesDisallowed_ThreeControls)
{
	uiControl *c, *d, *e;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	d = uiNewControl(testControlType(), NULL);
	e = uiNewControl(testControlType(), NULL);

	uiControlSetParent(c, d);
	uiControlSetParent(d, e);
	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot create a parent cycle");
	uiControlSetParent(e, c);
	endCheckProgrammerError(ctx);

	// cleanup
	uiControlSetParent(d, NULL);
	uiControlSetParent(c, NULL);
	uiControlFree(e);
	uiControlFree(d);
	uiControlFree(c);
}

Test(ControlCannotBeItsOwnParent)
{
	uiControl *c;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	ctx = beginCheckProgrammerError("uiControlSetParent(): cannot create a parent cycle");
	uiControlSetParent(c, c);
	endCheckProgrammerError(ctx);
	uiControlFree(c);
}

Test(GettingImplDataOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiControlImplData(): invalid null pointer for uiControl");
	uiControlImplData(NULL);
	endCheckProgrammerError(ctx);
}
