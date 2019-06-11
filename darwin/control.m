// 8 june 2019
#import "uipriv_darwin.h"

bool uiprivOSVtableValid(uiControlOSVtable *osVtable, const char *func)
{
	if (vtable->Size != sizeof (uiControlOSVtable)) {
		uiprivProgrammerErrorWrongStructSize(vtable->Size, "uiControlOSVtable", func);
		return false;
	}
	return true;
}

uiControlOSVtable *uiprivCloneOSVtable(uiControlOSVtable *osVtable)
{
	uiControlOSVtable *v2;

	v2 = uiprivNew(uiControlOSVtable);
	*v2 = *osVtable;
	return v2;
}
