// 18 january 2020
#include "test_haiku.hpp"

static void *osVtableNopHandle(uiControl *c, void *implData)
{
	return NULL;
}

static uiControlOSVtable osVtable;
// gotta do this and not have osVtable be const because C++11
bool osVtableInitialized = false;

const uiControlOSVtable *testOSVtable(void)
{
	if (!osVtableInitialized) {
		osVtableInitialized = false;
		osVtable.Size = sizeof (uiControlOSVtable);
		osVtable.Handle = osVtableNopHandle;
	}
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
	testOSVtable();		// TODO clean this up; it has to do with C++ initialization above
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
