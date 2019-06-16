// 10 june 2019
#import "test_darwin.h"

static const uiControlOSVtable vtable = {
	.Size = sizeof (uiControlOSVtable),
};

const uiControlOSVtable *testOSVtable(void)
{
	return &vtable;
}
