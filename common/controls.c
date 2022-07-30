// 8 june 2019
#include "uipriv.h"

struct controlType {
	uint32_t id;
	char *name;
	uiControlVtable vtable;
	uiControlOSVtable *osVtable;
	size_t implDataSize;
};

static int controlTypeCmp(const void *a, const void *b)
{
	const struct controlType *ca = (const struct controlType *) a;
	const struct controlType *cb = (const struct controlType *) b;

	if (ca->id < cb->id)
		return -1;
	if (ca->id > cb->id)
		return 1;
	return 0;
}

struct uiControl {
	uint32_t controlID;
	uint32_t typeID;
	struct controlType *type;
	void *implData;
	uiControl *parent;
};

static uiprivArray controlTypes = uiprivArrayStaticInit(struct controlType, 32, "uiControl type information");

#define controlTypeID UINT32_C(0x1F2E3C4D)

uint32_t uiControlType(void)
{
	if (!uiprivCheckInitializedAndThread())
		return 0;
	return controlTypeID;
}

static uint32_t nextControlID = UINT32_C(0x80000000);

uint32_t uiRegisterControlType(const char *name, const uiControlVtable *vtable, const uiControlOSVtable *osVtable, size_t implDataSize)
{
	struct controlType *ct;

	if (!uiprivCheckInitializedAndThread())
		return 0;
	if (name == NULL) {
		uiprivProgrammerErrorNullPointer("name", uiprivFunc);
		return 0;
	}
	if (vtable == NULL) {
		uiprivProgrammerErrorNullPointer("uiControlVtable", uiprivFunc);
		return 0;
	}
	if (vtable->Size != sizeof (uiControlVtable)) {
		uiprivProgrammerErrorWrongStructSize(vtable->Size, "uiControlVtable", uiprivFunc);
		return 0;
	}
#define checkMethod(method) \
	if (vtable->method == NULL) { \
		uiprivProgrammerErrorRequiredControlMethodMissing(name, "uiControlVtable", #method, uiprivFunc); \
		return 0; \
	}
	checkMethod(Init)
	checkMethod(Free)
	checkMethod(ParentChanging)
	checkMethod(ParentChanged)
#undef checkMethod

	if (osVtable == NULL) {
		uiprivProgrammerErrorNullPointer("uiControlOSVtable", uiprivFunc);
		return 0;
	}
	if (!uiprivOSVtableValid(name, osVtable, uiprivFunc))
		return 0;

	ct = (struct controlType *) uiprivArrayAppend(&controlTypes, 1);
	ct->id = nextControlID;
	nextControlID++;
	ct->name = uiprivStrdup(name);
	ct->vtable = *vtable;
	ct->osVtable = uiprivCloneOSVtable(osVtable);
	ct->implDataSize = implDataSize;
	return ct->id;
}

void *uiCheckControlType(void *c, uint32_t type)
{
	uiControl *cc = (uiControl *) c;
	struct controlType *got, *want;
	struct controlType key;

	if (!uiprivCheckInitializedAndThread())
		return NULL;
	if (c == NULL) {
		uiprivProgrammerErrorNullPointer("uiControl", uiprivFunc);
		return NULL;
	}
	if (cc->controlID != controlTypeID) {
		uiprivProgrammerErrorNotAControl(uiprivFunc);
		return NULL;
	}

	// now grab the type information for c itself
	// do this even if we were asked if this is a uiControl; we want to make absolutely sure this is a *real* uiControl
	memset(&key, 0, sizeof (struct controlType));
	key.id = cc->typeID;
	got = (struct controlType *) uiprivArrayBsearch(&controlTypes, &key, controlTypeCmp);
	if (got == NULL) {
		uiprivProgrammerErrorUnknownControlTypeUsed(cc->typeID, uiprivFunc);
		return NULL;
	}

	if (type == controlTypeID)
		// this is a real uiControl; no need to check further
		return c;

	// type isn't uiControlType(); make sure it is valid too
	memset(&key, 0, sizeof (struct controlType));
	key.id = type;
	want = (struct controlType *) uiprivArrayBsearch(&controlTypes, &key, controlTypeCmp);
	if (want == NULL) {
		uiprivProgrammerErrorUnknownControlTypeRequested(type, uiprivFunc);
		return NULL;
	}

	if (cc->typeID != type) {
		uiprivProgrammerErrorWrongControlType(got->name, want->name, uiprivFunc);
		return NULL;
	}
	return c;
}

#define callVtable(method, ...) ((*(method))(__VA_ARGS__))

uiControl *uiNewControl(uint32_t type, void *initData)
{
	uiControl *c;
	struct controlType *ct;
	struct controlType key;

	if (!uiprivCheckInitializedAndThread())
		return NULL;
	if (type == controlTypeID) {
		uiprivProgrammerErrorCannotCreateBaseControl(uiprivFunc);
		return NULL;
	}
	memset(&key, 0, sizeof (struct controlType));
	key.id = type;
	ct = (struct controlType *) uiprivArrayBsearch(&controlTypes, &key, controlTypeCmp);
	if (ct == NULL) {
		uiprivProgrammerErrorUnknownControlTypeRequested(type, uiprivFunc);
		return NULL;
	}

	c = uiprivNew(uiControl);
	c->controlID = controlTypeID;
	c->typeID = type;
	c->type = ct;
	if (ct->implDataSize != 0)
		c->implData = uiprivAlloc(ct->implDataSize, "uiControl implementation data");
	if (!callVtable(c->type->vtable.Init, c, c->implData, initData)) {
		uiprivProgrammerErrorInvalidControlInitData(ct->name, uiprivFunc);
		uiprivFree(c->implData);
		uiprivFree(c);
		return NULL;
	}
	return c;
}

void uiControlFree(uiControl *c)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	if (c == NULL) {
		uiprivProgrammerErrorNullPointer("uiControl", uiprivFunc);
		return;
	}
	if (c->parent != NULL) {
		uiprivProgrammerErrorControlHasParent(uiprivFunc);
		return;
	}

	callVtable(c->type->vtable.Free, c, c->implData);

	uiprivFree(c->implData);
	uiprivFree(c);
}

static bool parentHasCycle(uiControl *c, uiControl *parent)
{
	// TODO remember if this is the correct way to use a local uiprivArray
	uiprivArray parents;
	size_t i;

	if (parent == NULL)
		return false;
	if (parent == c)			// easy case
		return true;

	uiprivArrayInit(parents, uiControl *, 16, "uiControl parent list");
	// add these now, as they are counted as part of any cycles
	*((uiControl **) uiprivArrayAppend(&parents, 1)) = c;
	*((uiControl **) uiprivArrayAppend(&parents, 1)) = parent;
	for (c = parent->parent; c != NULL; c = c->parent) {
		// TODO this doesn't need to be sequential, but I don't imagine this list will ever be long enough to make it matter... yet
		for (i = 0; i < parents.len; i++)
			if (c == *uiprivArrayAt(parents, uiControl *, i)) {
				uiprivArrayFree(parents);
				return true;
			}
		// new parent; mark it as visited
		*((uiControl **) uiprivArrayAppend(&parents, 1)) = c;
	}

	uiprivArrayFree(parents);
	return false;
}

void uiControlSetParent(uiControl *c, uiControl *parent)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	if (c == NULL) {
		uiprivProgrammerErrorNullPointer("uiControl", uiprivFunc);
		return;
	}

	if (c->parent == NULL && parent == NULL) {
		uiprivProgrammerErrorReparenting("no", "no", uiprivFunc);
		return;
	}
	if (c->parent != NULL && parent != NULL) {
		uiprivProgrammerErrorReparenting("a", "another", uiprivFunc);
		return;
	}
	if (parentHasCycle(c, parent)) {
		uiprivProgrammerErrorControlParentCycle(uiprivFunc);
		return;
	}

	callVtable(c->type->vtable.ParentChanging, c, c->implData, c->parent);
	c->parent = parent;
	callVtable(c->type->vtable.ParentChanged, c, c->implData, c->parent);
}

void *uiControlImplData(uiControl *c)
{
	if (!uiprivCheckInitializedAndThread())
		return NULL;
	if (c == NULL) {
		uiprivProgrammerErrorNullPointer("uiControl", uiprivFunc);
		return NULL;
	}
	return c->implData;
}

uiControlOSVtable *uiprivControlOSVtable(uiControl *c)
{
	return c->type->osVtable;
}

uiControl *uiprivControlParent(uiControl *c)
{
	return c->parent;
}

static uiControl testHookControlWithInvalidControlMarker = {
	// use something other than 0 here to make it look like accidental real data
	.controlID = UINT32_C(0x5A5A5A5A),
};

uiControl *uiprivTestHookControlWithInvalidControlMarker(void)
{
	return &testHookControlWithInvalidControlMarker;
}

static uiControl testHookControlWithInvalidType = {
	.controlID = controlTypeID,
	.typeID = 0,
};

uiControl *uiprivTestHookControlWithInvalidType(void)
{
	return &testHookControlWithInvalidType;
}
