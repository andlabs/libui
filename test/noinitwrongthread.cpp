// 28 may 2019
#include "test.h"

static const struct checkErrorCase beforeInitCases[] = {
#define allcallsCase(f, ...) { #f "()", [](void) { f(__VA_ARGS__); }, "attempt to call " #f "() before uiInit()" },
#define allcallsIncludeQueueMain
#include "allcalls.h"
#undef allcallsIncludeQueueMain
#undef allcallsCase
	{ NULL, NULL, NULL },
};

testingTestInSet(beforeTests, FunctionsFailBeforeInit)
{
	checkProgrammerErrors(t, beforeInitCases);
}

static const struct checkErrorCase wrongThreadCases[] = {
#define allcallsCase(f, ...) { #f "()", [](void) { f(__VA_ARGS__); }, "attempt to call " #f "() on a thread other than the GUI thread" },
#include "allcalls.h"
#undef allcallsCase
	{ NULL, NULL, NULL },
};

testingTest(FunctionsFailOnWrongThread)
{
	checkProgrammerErrorsInThread(t, wrongThreadCases);
}
