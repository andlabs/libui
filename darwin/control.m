// 8 june 2019
#import "uipriv_darwin.h"

bool uiprivOSVtableValid(const uiControlOSVtable *osVtable, const char *func)
{
	if (osVtable->Size != sizeof (uiControlOSVtable)) {
		uiprivProgrammerErrorWrongStructSize(osVtable->Size, "uiControlOSVtable", func);
		return false;
	}
	return true;
}

uiControlOSVtable *uiprivCloneOSVtable(const uiControlOSVtable *osVtable)
{
	uiControlOSVtable *v2;

	v2 = uiprivNew(uiControlOSVtable);
	*v2 = *osVtable;
	return v2;
}
