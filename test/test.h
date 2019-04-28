// 28 april 2019
#include "../ui.h"
#include "testing.h"

// main.c
extern void timeoutMain(testingT *t, void *data);
#define timeout_uiMain(t, timeout, failNowOnError) \
	testingRunWithTimeout(t, timeout, \
		timeoutMain, NULL, \
		"uiMain()", failNowOnError);
