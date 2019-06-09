// 2 june 2019

// common {

#define uiprivProgrammerErrorNotInitialized(func) \
	uiprivProgrammerError("attempt to call %s() before uiInit()", \
		func)

#define uiprivProgrammerErrorWrongThread(func) \
	uiprivProgrammerError("attempt to call %s() on a thread other than the GUI thread", \
		func)

#define uiprivProgrammerErrorWrongStructSize(badSize, structName, func) \
	uiprivProgrammerError("wrong size %" uiprivSizetPrintf " for %s in %s()", \
		badSize, structName, func)

#define uiprivProgrammerErrorIndexOutOfRange(badIndex, func) \
	uiprivProgrammerError("index %d out of range in %s()", \
		badIndex, func)

#define uiprivProgrammerErrorNullPointer(paramDesc, func) \
	uiprivProgrammerError("invalid null pointer for %s passed into %s()", \
		paramDesc, func)

// }

// events {

#define uiprivProgrammerErrorEventHandlerNotFound(badID, func) \
	uiprivProgrammerError("event handler %d not found in %s()", \
		badID, func)

#define uiprivProgrammerErrorBadSenderForEvent(senderDesc, eventDesc, func) \
	uiprivProgrammerError("attempt to use a %s sender with a %s event in %s()", \
		senderDesc, eventDesc, func)

#define uiprivProgrammerErrorChangingEventDuringFire(func) \
	uiprivProgrammerError("attempt to change a uiEvent with %s() while it is firing", \
		func)

#define uiprivProgrammerErrorRecursiveEventFire() \
	uiprivProgrammerError("attempt to fire a uiEvent while it is already being fired")

#define uiprivProgrammerErrorFreeingInternalEvent() \
	uiprivProgrammerError("attempt to free a libui-provided uiEvent")

#define uiprivProgrammerErrorFreeingEventInUse() \
	uiprivProgrammerError("attempt to free a uiEvent that still has handlers registered")

#define uiprivProgrammerErrorInvalidatingGlobalEvent() \
	uiprivProgrammerError("attempt to call uiEventInvalidateSender() on a global uiEvent")

// }

// controls {

#define uiprivProgrammerErrorRequiredMethodMissing(typeName, tableType, methodName, func) \
	uiprivProgrammerError("%s: required %s method %s() missing in %s()", \
		typeName, tableType, methodName, func)

#define uiprivProgrammerErrorNotAControl(func) \
	uiprivProgrammerError("object passed in to %s() not a uiControl", \
		func)

#define uiprivProgrammerErrorUnknownTypeUsed(type, func) \
	uiprivProgrammerError("unknown type %" PRIu32 " found in uiControl passed to %s(); this is likely not a real uiControl or some data is corrupt", \
		type, func)

#define uiprivProgrammerErrorUnknownTypeRequested(type, func) \
	uiprivProgrammerError("unknown type %" PRIu32 " passed to %s()", \
		type, func)

#define uiprivProgrammerErrorWrongType(got, want, func) \
	uiprivProgrammerError("wrong type passed to %s(): got %s, want %s", \
		func, got, want)

#define uiprivProgrammerErrorCannotCreateBaseControl() \
	uiprivProgrammerError("cannot create a uiControl of type uiControl; you must use a specific control type")

#define uiprivProgrammerErrorInvalidControlInitData(type, func) \
	uiprivProgrammerError("invalid init data for %s in %s()", \
		type, func)

#define uiprivProgrammerErrorControlHasParent(func) \
	uiprivProgrammerError("control passed to %s() (which requires a control without a parent) has a parent", \
		func)

// }
