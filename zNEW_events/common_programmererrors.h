...
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
...
