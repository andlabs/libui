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

// main {

#define uiprivProgrammerErrorMultipleCalls(func) \
	uiprivProgrammerError("%s(): attempt to call more than once", func)

#define uiprivProgrammerErrorBadInitOptions(func) \
	uiprivProgrammerError("%s(): invalid uiInitOptions passed", func)

#define uiprivProgrammerErrorQuitBeforeMain(func) \
	uiprivProgrammerError("%s(): attempt to call before uiMain()", func)

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

#define uiprivProgrammerErrorReparenting(current, next, func) \
	uiprivProgrammerError("%s(): cannot set a control with %s parent to have %s parent", \
		func, current, next)

#define uiprivProgrammerErrorControlParentCycle(func) \
	uiprivProgrammerError("%s(): cannot create a parent cycle", func)

// }

// windows {

// TODO have any parameters, such as what the window is and what the parent is? to add func we'll need to carry that out from uiControlSetParent() (which means exposing that in the API)
#define uiprivProgrammerErrorCannotHaveWindowsAsChildren() \
	uiprivProgrammerError("cannot set a uiWindow as the child of another uiControl")

// for Windows only
#define uiprivProgrammerErrorCannotCallSetControlPosOnWindow() \
	uiprivProgrammerError("cannot call uiWindowsControlSetControlPos() on a uiWindow")

// }
