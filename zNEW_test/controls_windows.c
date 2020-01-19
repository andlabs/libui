// 10 june 2019
#include "test_windows.h"

static const uiControlOSVtable vtable = {
	.Size = sizeof (uiControlOSVtable),
};

const uiControlOSVtable *testOSVtable(void)
{
	return &vtable;
}
