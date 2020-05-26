// 8 june 2019
#import "uipriv_darwin.h"

bool uiprivOSVtableValid(const char *name, const uiControlOSVtable *osVtable, const char *func)
{
	if (osVtable->Size != sizeof (uiControlOSVtable)) {
		uiprivProgrammerErrorWrongStructSize(osVtable->Size, "uiControlOSVtable", func);
		return false;
	}
#define checkMethod(method) \
	if (osVtable->method == NULL) { \
		uiprivProgrammerErrorRequiredControlMethodMissing(name, "uiControlOSVtable", #method, func); \
		return 0; \
	}
	checkMethod(Handle)
	return true;
}

uiControlOSVtable *uiprivCloneOSVtable(const uiControlOSVtable *osVtable)
{
	uiControlOSVtable *v2;

	v2 = uiprivNew(uiControlOSVtable);
	*v2 = *osVtable;
	return v2;
}

#define callVtable(method, ...) ((*(method))(__VA_ARGS__))

id uiDarwinControlHandle(uiControl *c)
{
	uiControlOSVtable *osVtable;

	if (!uiprivCheckInitializedAndThread())
		return nil;
	if (c == NULL) {
		uiprivProgrammerErrorNullPointer("uiControl", uiprivFunc);
		return nil;
	}
	osVtable = uiprivControlOSVtable(c);
	return callVtable(osVtable->Handle, c, uiControlImplData(c));
}
