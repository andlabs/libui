// 11 june 2019
#include "test.h"

static const struct checkErrorCase cases[] = {
	{
		"uiRegisterControlType() with NULL name",
		[](void) {
			uiRegisterControlType(NULL, NULL, NULL, 0);
		},
		"uiRegisterControlType(): invalid null pointer for name",
	},
	{
		"uiRegisterControlType() with NULL vtable",
		[](void) {
			uiRegisterControlType("name", NULL, NULL, 0);
		},
		"uiRegisterControlType(): invalid null pointer for uiControlVtable",
	},
	{
		"uiRegisterControlType() with vtable with wrong size",
		[](void) {
			uiControlVtable vtable;
	
			memset(&vtable, 0, sizeof (uiControlVtable));
			vtable.Size = 1;
			uiRegisterControlType("name", &vtable, NULL, 0);
		},
		"uiRegisterControlType(): wrong size 1 for uiControlVtable",
	},
#define checkVtableMethod(name) \
	{ \
		"uiRegisterControlType() with NULL " #name " method", \
		[](void) { \
			uiControlVtable vtable; \
			vtable = *testVtable(); \
			vtable.name = NULL; \
			uiRegisterControlType("name", &vtable, NULL, 0); \
		}, \
		"uiRegisterControlType(): required uiControlVtable method " #name "() missing for uiControl type name", \
	}
	checkVtableMethod(Init),
	checkVtableMethod(Free),
	{
		"uiRegisterControlType() with NULL OS vtable",
		[](void) {
			uiRegisterControlType("name", testVtable(), NULL, 0);
		},
		"uiRegisterControlType(): invalid null pointer for uiControlOSVtable",
	},
	// OS vtable sizes are tested per-OS
	{ NULL, NULL, NULL },
};

testingTest(ControlErrors)
{
	checkProgrammerErrors(t, cases);
}
