// 11 june 2019
#include "test.h"

// TODO once this is filled in, verify against the documentation that this is all the error cases
// TODO add an internal marker to uiEvent as well, just to be safe?

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
		"uiCheckControlType(): invalid null pointer for uiControl",
	},
	{
		"uiCheckControlType() with non-control",
		[](void) {
			// TODO make sure this is fine; if not, just use the following check instead
			static char buf[] = "this is not a uiControl but is big enough to at the very least not cause a problem with UB hopefully";

			uiCheckControlType(buf, uiControlType());
		},
		"uiCheckControlType(): object passed in not a uiControl",
	},
	{
		"uiCheckControlType() with uiControl without uiControlType() marker",
		[](void) {
			uiCheckControlType(uiprivTestHookControlWithInvalidControlMarker(), uiControlType());
		},
		"uiCheckControlType(): object passed in not a uiControl",
	},
	{
		"uiCheckControlType() with a control with an unknown type",
		[](void) {
			uiCheckControlType(uiprivTestHookControlWithInvalidType(), testControlType);
		},
		"uiCheckControlType(): unknown uiControl type 5 found in uiControl (this is likely not a real uiControl or some data is corrupt)",
	},
	{
		"uiCheckControlType() with a control with an unknown type, even with uiControlType()",
		[](void) {
			uiCheckControlType(uiprivTestHookControlWithInvalidType(), uiControlType());
		},
		"uiCheckControlType(): unknown uiControl type 5 found in uiControl (this is likely not a real uiControl or some data is corrupt)",
	},
	{
		"uiCheckControlType() asking for an unknown control type",
		[](void) {
			uiControl *c;

			c = uiNewControl(testControlType, NULL);
			uiCheckControlType(c, 5);
			uiControlFree(c);
		},
		"uiCheckControlType(): unknown uiControl type 5 requested",
	},
	{
		"uiCheckControlType() with a type mismatch",
		[](void) {
			uiControl *c;

			c = uiNewControl(testControlType, NULL);
			uiCheckControlType(c, testControlType2);
			uiControlFree(c);
		},
		"uiCheckControlType(): wrong uiControl type passed: got TestControl, want TestControl2",
	},

	{
		"uiNewControl() with uiControlType()",
		[](void) {
			uiNewControl(uiControlType(), NULL);
		},
		"uiNewControl(): uiControlType() passed in when specific control type needed",
	},
	{
		"uiNewControl() with an unknown control type",
		[](void) {
			uiNewControl(5, NULL);
		},
		"uiNewControl(): unknown uiControl type 5 requested",
	},
	{
		"uiNewControl() with Init() failing",
		[](void) {
			uiNewControl(testControlType, testControlFailInit);
		},
		"uiNewControl(): invalid init data for TestControl",
	},

	{
		"uiControlFree() with a NULL uiControl",
		[](void) {
			uiControlFree(NULL);
		},
		"uiControlFree(): invalid null pointer for uiControl",
	},
	{
		"uiControlFree() with a uiControl that still has a parent",
		[](void) {
			// TODO
		},
		"TODO",
	},

	{
		"uiControlImplData() with a NULL uiControl",
		[](void) {
			uiControlImplData(NULL);
		},
		"uiControlImplData(): invalid null pointer for uiControl",
	},
	{ NULL, NULL, NULL },
};

testingTest(ControlErrors)
{
	checkProgrammerErrors(t, casesBeforeOSVtable);
	// OS vtable sizes are tested per-OS
	checkProgrammerErrors(t, casesAfterOSVtable);
}
