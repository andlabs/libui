// 28 may 2019
#include "test.h"

#define allcallsCase(f, ...) \
	void doCase ## f(void *data) \
	{ \
		f(__VA_ARGS__); \
	}
allcallsCase(uiQueueMain, NULL, NULL)
#include "allcalls.h"
#undef allcallsCase

static const struct {
	const char *name;
	void (*f)(void *data);
	const char *beforeInitWant;
	const char *wrongThreadWant;
} allCases[] = {
#define allcallsCase(f, ...) { #f, doCase ## f, \
	"attempt to call " #f "() before uiInit()", \
	allcallsThread(#f), \
},
#define allcallsThread(f) NULL
allcallsCase(uiQueueMain, NULL, NULL)
#undef allcallsThread
#define allcallsThread(f) "attempt to call " f "() on a thread other than the GUI thread"
#include "allcalls.h"
#undef allcallsCase
	{ NULL, NULL, NULL, NULL },
};

testingTestInSet(beforeTests, FunctionsFailBeforeInit)
{
	size_t i;

	for (i = 0; allCases[i].name != NULL; i++) {
		if (allCases[i].beforeInitWant == NULL)
			continue;
		checkProgrammerError(t, allCases[i].name, allCases[i].f, NULL, allCases[i].beforeInitWant);
	}
}

testingTest(FunctionsFailOnWrongThread)
{
	size_t i;

	for (i = 0; allCases[i].name != NULL; i++) {
		if (allCases[i].wrongThreadWant == NULL)
			continue;
		checkProgrammerErrorInThread(t, allCases[i].name, allCases[i].f, NULL, allCases[i].wrongThreadWant);
	}
}
