// 18 january 2020
#include "test_haiku.hpp"

static void *osVtableNopHandle(uiControl *c, void *implData)
{
	return NULL;
}

static const uiControlOSVtable osVtable = [](void) {
	uiControlOSVtable vt;

	memset(&vt, 0, sizeof (uiControlOSVtable));
	vt.Size = sizeof (uiControlOSVtable);
	vt.Handle = osVtableNopHandle;
	return vt;
}();

const uiControlOSVtable *testOSVtable(void)
{
	return &osVtable;
}

Test(WrongControlOSVtableSizeIsProgrammerError)
{
	uiControlVtable vtable;
	uiControlOSVtable osvt;
	void *ctx;

	testControlLoadNopVtable(&vtable);
	ctx = beginCheckProgrammerError("uiRegisterControlType(): wrong size 1 for uiControlOSVtable");
	memset(&osvt, 0, sizeof (uiControlOSVtable));
	osvt.Size = 1;
	uiRegisterControlType("name", &vtable, &osvt, 0);
	endCheckProgrammerError(ctx);
}

Test(ControlOSVtableWithMissingHandleMethodIsProgrammerError)
{
	uiControlVtable vtable;
	uiControlOSVtable osvt;
	void *ctx;

	testControlLoadNopVtable(&vtable);
	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlOSVtable method Handle() missing for uiControl type name");
	osvt = osVtable;
	osvt.Handle = NULL;
	uiRegisterControlType("name", &vtable, &osvt, 0);
	endCheckProgrammerError(ctx);
}

Test(GettingHaikuHandleOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiHaikuControlHandle(): invalid null pointer for uiControl");
	uiHaikuControlHandle(NULL);
	endCheckProgrammerError(ctx);
}
