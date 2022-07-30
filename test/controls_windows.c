// 10 june 2019
#include "test_windows.h"

static HWND osVtableNopHandle(uiControl *c, void *implData)
{
	return NULL;
}

static HWND osVtableNopParentHandleForChild(uiControl *c, void *implData, uiControl *child)
{
	return NULL;
}

HRESULT osVtableNopSetControlPos(uiControl *c, void *implData, const RECT *r)
{
	return S_OK;
}

static const uiControlOSVtable osVtable = {
	.Size = sizeof (uiControlOSVtable),
	.Handle = osVtableNopHandle,
	.ParentHandleForChild = osVtableNopParentHandleForChild,
	.SetControlPos = osVtableNopSetControlPos,
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

Test(ControlOSVtableWithMissingParentHandleForChildMethodIsProgrammerError)
{
	uiControlVtable vtable;
	uiControlOSVtable osvt;
	void *ctx;

	testControlLoadNopVtable(&vtable);
	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlOSVtable method ParentHandleForChild() missing for uiControl type name");
	osvt = osVtable;
	osvt.ParentHandleForChild = NULL;
	uiRegisterControlType("name", &vtable, &osvt, 0);
	endCheckProgrammerError(ctx);
}

Test(ControlOSVtableWithMissingSetControlPosMethodIsProgrammerError)
{
	uiControlVtable vtable;
	uiControlOSVtable osvt;
	void *ctx;

	testControlLoadNopVtable(&vtable);
	ctx = beginCheckProgrammerError("uiRegisterControlType(): required uiControlOSVtable method SetControlPos() missing for uiControl type name");
	osvt = osVtable;
	osvt.SetControlPos = NULL;
	uiRegisterControlType("name", &vtable, &osvt, 0);
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

	ctx = beginCheckProgrammerError("uiWindowsControlHandle(): invalid null pointer for uiControl");
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

Test(SettingWindowsControlPosOfNullControlIsProgrammerError)
{
#if 0
	// TODO
	uiControl *c;
	void *ctx;

	ctx = beginCheckProgrammerError("uiWindowsControlSetControlPos(): invalid null pointer for RECT");
	uiWindowsControlSetControlPos(c, NULL);
	endCheckProgrammerError(ctx);
#endif
}

// TODO uiWindowsSetControlHandlePos errors
