...
#endif

allcallsCase(uiNewEvent, NULL)
allcallsCase(uiEventAddHandler, NULL, NULL, NULL, NULL)
allcallsCase(uiEventDeleteHandler, NULL, 0)
allcallsCase(uiEventFire, NULL, NULL, NULL)
allcallsCase(uiEventHandlerBlocked, NULL, 0)
allcallsCase(uiEventSetHandlerBlocked, NULL, 0, false)
allcallsCase(uiEventInvalidateSender, NULL, NULL)

allcallsCase(uiControlType, /* no arguments */)
...
