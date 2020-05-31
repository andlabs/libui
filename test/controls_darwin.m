// 10 june 2019
#import "test_darwin.h"

static id osVtableNopHandle(uiControl *c, void *implData)
{
	return nil;
}

static const uiControlOSVtable osVtable = {
	.Size = sizeof (uiControlOSVtable),
	.Handle = osVtableNopHandle,
};

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

Test(GettingDarwinHandleOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiDarwinControlHandle(): invalid null pointer for uiControl");
	uiDarwinControlHandle(NULL);
	endCheckProgrammerError(ctx);
}
