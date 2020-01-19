// 18 january 2020
#include "test_haiku.h"

static const uiControlOSVtable vtable = {
	.Size = sizeof (uiControlOSVtable),
};

const uiControlOSVtable *testOSVtable(void)
{
	return &vtable;
}
