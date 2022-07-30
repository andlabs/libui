// 18 january 2020
#include "test_haiku.hpp"

static void *testControlHandle(uiControl *c, void *implData)
{
	struct testControlImplData *ti = (struct testControlImplData *) implData;

	if (ti->realOSVtable != NULL && ti->realOSVtable->Handle != NULL)
		return (*(ti->realOSVtable->Handle))(c, ti->realImplData);
	return NULL;
}

static const uiControlOSVtable osVtable = [](void) {
	uiControlOSVtable vt;

	memset(&vt, 0, sizeof (uiControlOSVtable));
	vt.Size = sizeof (uiControlOSVtable);
	vt.Handle = testControlHandle;
	return vt;
}();

const uiControlOSVtable *testControlOSVtable(void)
{
	return &osVtable;
}

Test(WrongControlOSVtableSizeIsProgrammerError)
{
	uiControlOSVtable osvt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): wrong size 1 for uiControlOSVtable");
	memset(&osvt, 0, sizeof (uiControlOSVtable));
	osvt.Size = 1;
	uiRegisterControlType("name", testControlVtable(), &osvt, 0);
	endCheckProgrammerError(ctx);
}

Test(ControlOSVtableWithMissingHandleMethodIsProgrammerError)
{
	uiControlOSVtable osvt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlOSVtable method Handle() missing for uiControl type name");
	osvt = osVtable;
	osvt.Handle = NULL;
	uiRegisterControlType("name", testControlVtable(), &osvt, 0);
	endCheckProgrammerError(ctx);
}

Test(GettingHaikuHandleOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiHaikuControlHandle(): invalid null pointer for uiControl");
	uiHaikuControlHandle(NULL);
	endCheckProgrammerError(ctx);
}
