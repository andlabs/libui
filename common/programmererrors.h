// 2 june 2019

#define uiprivProgrammerErrorNotInitialized(func) \
	uiprivProgrammerError("attempt to call %s() before uiInit()", \
		func)

#define uiprivProgrammerErrorWrongThread(func) \
	uiprivProgrammerError("attempt to call %s() on a thread other than the GUI thread", \
		func)

#define uiprivProgrammerErrorWrongStructSize(badSize, structName) \
	uiprivProgrammerError("wrong size %" uiprivSizetPrintf " for %s", \
		badSize, structName)

#define uiprivProgrammerErrorIndexOutOfRange(badIndex, func) \
	uiprivProgrammerError("index %d out of range in %s()", \
		badIndex, func)

#define uiprivProgrammerErrorNullPointer(paramDesc, func) \
	uiprivProgrammerError("invalid null pointer for %s passed into %s()", \
		paramDesc, func)

#define uiprivProgrammerErrorIntIDNotFound(idDesc, badID, func) \
	uiprivProgrammerError("%s identifier %d not found in %s()", \
		idDesc, badID, func)

// TODO type mismatch

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
