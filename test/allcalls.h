// 28 may 2019

// This file should NOT have include guards as it is intended to be included more than once; see noinitwrongthread.c for details.
// TODO pass in file and line properly

allcallsCase(uiMain, /* no arguments */)
allcallsCase(uiQuit, /* no arguments */)

allcallsCase(uiNewEvent, NULL)
allcallsCase(uiEventAddHandler, NULL, NULL, NULL, NULL)
allcallsCase(uiEventDeleteHandler, NULL, 0)
allcallsCase(uiEventFire, NULL, NULL, NULL)
allcallsCase(uiEventHandlerBlocked, NULL, 0)
allcallsCase(uiEventSetHandlerBlocked, NULL, 0, false)
