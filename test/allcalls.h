// 28 may 2019

// This file should NOT have include guards as it is intended to be included more than once; see noinitwrongthread.c for details.

allcallsCase(uiMain, /* no arguments */)
allcallsCase(uiQuit, /* no arguments */)
#ifdef allcallsIncludeQueueMain
allcallsCase(uiQueueMain, NULL, NULL)
#endif

allcallsCase(uiNewEvent, NULL)
allcallsCase(uiEventAddHandler, NULL, NULL, NULL, NULL)
allcallsCase(uiEventDeleteHandler, NULL, 0)
allcallsCase(uiEventFire, NULL, NULL, NULL)
allcallsCase(uiEventHandlerBlocked, NULL, 0)
allcallsCase(uiEventSetHandlerBlocked, NULL, 0, false)
allcallsCase(uiEventInvalidateSender, NULL, NULL)

allcallsCase(uiControlType, /* no arguments */)

allcallsCase(uiRegisterControlType, NULL, NULL, NULL, 0)
allcallsCase(uiCheckControlType, NULL, 0)

allcallsCase(uiNewControl, 0, NULL)
allcallsCase(uiControlFree, NULL)
allcallsCase(uiControlSetParent, NULL, NULL)
allcallsCase(uiControlImplData, NULL)
allcallsCase(uiControlOnFree, /* no arguments */)
