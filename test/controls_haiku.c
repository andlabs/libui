// 18 january 2020
#include "test_haiku.h"

static const uiControlOSVtable osVtable = {
	.Size = sizeof (uiControlOSVtable),
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
