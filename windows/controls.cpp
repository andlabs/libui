// 8 june 2019
#include "uipriv_windows.hpp"

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
	checkMethod(ParentHandleForChild)
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

HWND uiWindowsControlHandle(uiControl *c)
{
	uiControlOSVtable *osVtable;

	if (!uiprivCheckInitializedAndThread())
		return NULL;
	if (c == NULL) {
		uiprivProgrammerErrorNullPointer("uiControl", uiprivFunc);
		return NULL;
	}
	osVtable = uiprivControlOSVtable(c);
	return callVtable(osVtable->Handle, c, uiControlImplData(c));
}

HWND uiWindowsControlParentHandle(uiControl *c)
{
	uiControl *parent;
	uiControlOSVtable *parentVtable;

	if (!uiprivCheckInitializedAndThread())
		return NULL;
	if (c == NULL) {
		uiprivProgrammerErrorNullPointer("uiControl", uiprivFunc);
		return NULL;
	}
	parent = uiControlParent(c);
	if (parent == NULL)
		return NULL;
	parentVtable = uiprivControlOSVtable(parent);
	return callVtable(parentVtable->ParentHandleForChild, parent, uiControlImplData(parent), c);
}
