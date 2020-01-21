// 28 may 2019
#include "test.h"

#define allcallsCaseFuncName(f) allcallsCaseFunc ## f
#define allcallsCase(f, ...) static void allcallsCaseFuncName(f)(void) { f(__VA_ARGS__); }
#define allcallsIncludeQueueMain
#include "allcalls.h"
#undef allcallsIncludeQueueMain
#undef allcallsCase

static const struct checkErrorCase beforeInitCases[] = {
#define allcallsCase(f, ...) { #f "()", allcallsCaseFuncName(f), "attempt to call " #f "() before uiInit()" },
#define allcallsIncludeQueueMain
#include "allcalls.h"
#undef allcallsIncludeQueueMain
#undef allcallsCase
	{ NULL, NULL, NULL },
};

TestNoInit(FunctionsFailBeforeInit)
{
	checkProgrammerErrors(beforeInitCases);
}

static const struct checkErrorCase wrongThreadCases[] = {
#define allcallsCase(f, ...) { #f "()", allcallsCaseFuncName(f), "attempt to call " #f "() on a thread other than the GUI thread" },
#include "allcalls.h"
#undef allcallsCase
	{ NULL, NULL, NULL },
};

Test(FunctionsFailOnWrongThread)
{
	checkProgrammerErrorsInThread(wrongThreadCases);
}
