// 10 june 2019
#import "test_darwin.h"

static id testControlHandle(uiControl *c, void *implData)
{
	struct testControlImplData *ti = (struct testControlImplData *) implData;

	if (ti->realOSVtable != NULL && ti->realOSVtable->Handle != NULL)
		return (*(ti->realOSVtable->Handle))(c, ti->realImplData);
	return nil;
}

static const uiControlOSVtable osVtable = {
	.Size = sizeof (uiControlOSVtable),
	.Handle = testControlHandle,
};

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

Test(GettingDarwinHandleOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiDarwinControlHandle(): invalid null pointer for uiControl");
	uiDarwinControlHandle(NULL);
	endCheckProgrammerError(ctx);
}
