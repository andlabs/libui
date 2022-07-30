// 10 june 2019
#include "test_windows.h"

static HWND testControlHandle(uiControl *c, void *implData)
{
	struct testControlImplData *ti = (struct testControlImplData *) implData;

	if (ti->realOSVtable != NULL && ti->realOSVtable->Handle != NULL)
		return (*(ti->realOSVtable->Handle))(c, ti->realImplData);
	return NULL;
}

static HWND testControlParentHandleForChild(uiControl *c, void *implData, uiControl *child)
{
	struct testControlImplData *ti = (struct testControlImplData *) implData;

	if (ti->realOSVtable != NULL && ti->realOSVtable->ParentHandleForChild != NULL)
		return (*(ti->realOSVtable->ParentHandleForChild))(c, ti->realImplData, child);
	return NULL;
}

static HRESULT testControlSetControlPos(uiControl *c, void *implData, const RECT *r)
{
	struct testControlImplData *ti = (struct testControlImplData *) implData;

	if (ti->realOSVtable != NULL && ti->realOSVtable->SetControlPos != NULL)
		return (*(ti->realOSVtable->SetControlPos))(c, ti->realImplData, r);
	return S_OK;
}

static const uiControlOSVtable osVtable = {
	.Size = sizeof (uiControlOSVtable),
	.Handle = testControlHandle,
	.ParentHandleForChild = testControlParentHandleForChild,
	.SetControlPos = testControlSetControlPos,
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

Test(ControlOSVtableWithMissingParentHandleForChildMethodIsProgrammerError)
{
	uiControlOSVtable osvt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlOSVtable method ParentHandleForChild() missing for uiControl type name");
	osvt = osVtable;
	osvt.ParentHandleForChild = NULL;
	uiRegisterControlType("name", testControlVtable(), &osvt, 0);
	endCheckProgrammerError(ctx);
}

Test(ControlOSVtableWithMissingSetControlPosMethodIsProgrammerError)
{
	uiControlOSVtable osvt;
	void *ctx;

	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlOSVtable method SetControlPos() missing for uiControl type name");
	osvt = osVtable;
	osvt.SetControlPos = NULL;
	uiRegisterControlType("name", testControlVtable(), &osvt, 0);
	endCheckProgrammerError(ctx);
}

Test(GettingWindowsHandleOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiWindowsControlHandle(): invalid null pointer for uiControl");
	uiWindowsControlHandle(NULL);
	endCheckProgrammerError(ctx);
}

Test(GettingWindowsParentHandleOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiWindowsControlParentHandle(): invalid null pointer for uiControl");
	uiWindowsControlParentHandle(NULL);
	endCheckProgrammerError(ctx);
}

Test(SettingWindowsControlPosOfNullControlIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiWindowsControlSetControlPos(): invalid null pointer for uiControl");
	uiWindowsControlSetControlPos(NULL, NULL);
	endCheckProgrammerError(ctx);
}

Test(SettingWindowsControlPosToNullRectIsProgrammerError)
{
	uiControl *c;
	void *ctx;

	c = uiNewControl(testControlType(), NULL);
	ctx = beginCheckProgrammerError("uiWindowsControlSetControlPos(): invalid null pointer for RECT");
	uiWindowsControlSetControlPos(c, NULL);
	endCheckProgrammerError(ctx);
	uiControlFree(c);
}

// TODO uiWindowsSetControlHandlePos errors
