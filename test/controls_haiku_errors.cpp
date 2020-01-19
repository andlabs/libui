// 18 january 2020
#include "test_haiku.h"

const struct checkErrorCase controlOSVtableCases[] = {
	{
		"uiRegisterControlType() with OS vtable with wrong size",
		[](void) {
			uiControlOSVtable vtable;
		
			memset(&vtable, 0, sizeof (uiControlOSVtable));
			vtable.Size = 1;
			uiRegisterControlType("name", testVtable(), &vtable, 0);
		},
		"uiRegisterControlType(): wrong size 1 for uiControlOSVtable",
	},
	{ NULL, NULL, NULL },
};
