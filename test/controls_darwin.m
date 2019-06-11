// 10 june 2019
#import "test_darwin.h"

uiControlOSVtable *allocOSVtableFull(testingT *t, const char *file, long line)
{
	uiControlOSVtable *v;

	v = (uiControlOSVtable *) malloc(sizeof (uiControlOSVtable));
	if (v == NULL)
		testingTFatalfFull(t, file, line, "memory exhausted allocating uiControlOSVtable");
	memset(v, 0, sizeof (uiControlOSVtable));
	v->Size = sizeof (uiControlOSVtable);
	return v;
}
