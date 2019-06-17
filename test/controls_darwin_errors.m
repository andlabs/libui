// 16 june 2019
#import "test_darwin.h"

// TODO move this back to C++ once https://github.com/mesonbuild/meson/issues/5495 is resolved

static void doRCTWrongSizeTest(void)
{
	uiControlOSVtable vtable;

	memset(&vtable, 0, sizeof (uiControlOSVtable));
	vtable.Size = 1;
	uiRegisterControlType("name", testVtable(), &vtable, 0);
}

const struct checkErrorCase controlOSVtableCases[] = {
	{
		"uiRegisterControlType() with OS vtable with wrong size",
		doRCTWrongSizeTest,
		"uiRegisterControlType(): wrong size 1 for uiControlOSVtable",
	},
	{ NULL, NULL, NULL },
};
