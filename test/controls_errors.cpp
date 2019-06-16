// 11 june 2019
#include "test.h"

static const struct checkErrorCase casesBeforeOSVtable[] = {
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
	{ NULL, NULL, NULL },
};

static const struct checkErrorCase casesAfterOSVtable[] = {
	{
		"uiCheckControlType() with a NULL pointer",
		[](void) {
			uiCheckControlType(NULL, uiControlType());
		},
		"TODO",
	},
	{
		"uiCheckControlType() with non-control",
		[](void) {
			static char buf[] = "this is not a uiControl";

			uiCheckControlType(buf, uiControlType());
		},
		"TODO",
	},
	{
		"uiCheckControlType() with a control with an unknown type",
		[](void) {
			// TODO
		},
		"TODO",
	},
	{
		"uiCheckControlType() asking for an unknown control type",
		[](void) {
			// TODO
		},
		"TODO",
	},
	{
		"uiCheckControlType() with a type mismatch",
		[](void) {
			// TODO
		},
		"TODO",
	},

	{
		"uiNewControl() with uiControlType()",
		[](void) {
			uiNewControl(uiControlType(), NULL);
		},
		"TODO",
	},
	{
		"uiNewControl() with an unknown control type",
		[](void) {
			uiNewControl(5, NULL);
		},
		"TODO",
	},
	// TODO have Init() fail

	// TODO uiControlFree()

	{
		"uiControlImplData() with a NULL uiControl",
		[](void) {
			uiControlImplData(NULL);
		},
		"TODO",
	},
	{ NULL, NULL, NULL },
};

testingTest(ControlErrors)
{
	checkProgrammerErrors(t, casesBeforeOSVtable);
	// OS vtable sizes are tested per-OS
	checkProgrammerErrors(t, casesAfterOSVtable);
}
