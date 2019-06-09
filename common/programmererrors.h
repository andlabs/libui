// 2 june 2019

// common {

#define uiprivProgrammerErrorNotInitialized(func) \
	uiprivProgrammerError("attempt to call %s() before uiInit()", \
		func)

#define uiprivProgrammerErrorWrongThread(func) \
	uiprivProgrammerError("attempt to call %s() on a thread other than the GUI thread", \
		func)

#define uiprivProgrammerErrorWrongStructSize(badSize, structName, func) \
	uiprivProgrammerError("%s(): wrong size %" uiprivSizetPrintf " for %s", \
		func, badSize, structName)

#define uiprivProgrammerErrorIndexOutOfRange(badIndex, func) \
	uiprivProgrammerError("%s(): index %d out of range", \
		func, badIndex)

#define uiprivProgrammerErrorNullPointer(paramDesc, func) \
	uiprivProgrammerError("%s(): invalid null pointer for %s", \
		func, paramDesc)

// }

// events {

#define uiprivProgrammerErrorEventHandlerNotFound(badID, func) \
	uiprivProgrammerError("%s(): event handler %d not found", \
		func, badID)

#define uiprivProgrammerErrorBadSenderForEvent(senderDesc, eventDesc, func) \
	uiprivProgrammerError("%s(): can't use a %s sender with a %s event", \
		func, senderDesc, eventDesc)

#define uiprivProgrammerErrorChangingEventDuringFire(func) \
	uiprivProgrammerError("%s(): can't change a uiEvent while it is firing", \
		func)

#define uiprivProgrammerErrorRecursiveEventFire(func) \
	uiprivProgrammerError("%s(): can't recursively fire a uiEvent", \
		func)

#define uiprivProgrammerErrorFreeingInternalEvent(func) \
	uiprivProgrammerError("%s(): can't free a libui-provided event", \
		func)

#define uiprivProgrammerErrorFreeingEventInUse(func) \
	uiprivProgrammerError("%s(): can't free event that still has handlers registered", \
		func)

#define uiprivProgrammerErrorInvalidatingGlobalEvent(func) \
	uiprivProgrammerError("%s(): can't invalidate a global event", \
		func)

// }

// controls {

#define uiprivProgrammerErrorRequiredControlMethodMissing(typeName, tableType, methodName, func) \
	uiprivProgrammerError("%s(): required %s method %s() missing for uiControl type %s", \
		func, tableType, methodName, typeName)

#define uiprivProgrammerErrorNotAControl(func) \
	uiprivProgrammerError("%s(): object passed in not a uiControl", \
		func)

#define uiprivProgrammerErrorUnknownControlTypeUsed(type, func) \
	uiprivProgrammerError("%s(): unknown uiControl type %" PRIu32 " found in uiControl (this is likely not a real uiControl or some data is corrupt)", \
		func, type)

#define uiprivProgrammerErrorUnknownControlTypeRequested(type, func) \
	uiprivProgrammerError("%s(): unknown uiControl type %" PRIu32 " requested", \
		func, type)

#define uiprivProgrammerErrorWrongControlType(got, want, func) \
	uiprivProgrammerError("%s(): wrong uiControl type passed: got %s, want %s", \
		func, got, want)

#define uiprivProgrammerErrorCannotCreateBaseControl(func) \
	uiprivProgrammerError("%s(): uiControlType() passed in when specific control type needed", \
		func)

#define uiprivProgrammerErrorInvalidControlInitData(type, func) \
	uiprivProgrammerError("%s(): invalid init data for %s", \
		func, type)

#define uiprivProgrammerErrorControlHasParent(func) \
	uiprivProgrammerError("%s(): cannot be called on a control with has a parent", \
		func)

// }
