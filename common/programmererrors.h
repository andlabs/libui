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

#define uiprivProgrammerErrorRecursiveEventFire(func) \
	uiprivProgrammerError("attempt to fire a uiEvent while it is already being fired in %s()", \
		func)

#define uiprivProgrammerErrorFreeingInternalEvent(func) \
	uiprivProgrammerError("attempt to free a libui-provided uiEvent", \
		func)

#define uiprivProgrammerErrorFreeingEventInUse(func) \
	uiprivProgrammerError("attempt to free a uiEvent that still has handlers registered", \
		func)

#define uiprivProgrammerErrorInvalidatingGlobalEvent(func) \
	uiprivProgrammerError("attempt to call uiEventInvalidateSender() on a global uiEvent", \
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
