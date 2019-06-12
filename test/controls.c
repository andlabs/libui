// 8 june 2019
#include "test.h"

struct testImplData {
	bool initCalled;
	bool *freeCalled;
	bool testMethodCalled;
};

static bool testVtableInit(uiControl *c, void *implData, void *initData)
{
	return true;
}

static void testVtableFree(uiControl *c, void *implData)
{
	// do nothing
}

static void testVtableTestMethod(uiControl *c, void *implData)
{
	// do nothing
}

static void createTestVtable(uiControlVtable *vtable)
{
	memset(&vtable, 0, sizeof (uiControlVtable));
	vtable.Size = sizeof (uiControlVtable);
	vtable.Init = testVtableInit;
	vtable.Free = testVtableFree;
}

struct checkControlErrorsParams {
	const char *namePlaceholder;
	uiControlVtable *vtablePlaceholder;
	uiControlOSVtable *osVtablePlaceholder;
	size_t implDataSizePlaceholder;
	uiControlVtable *badSizeVtable;
};

// TODO clean up these macros
#define checkCat(a, b) a ## b
#define checkErrorCaseFull(line, call, msgWant) \
	static void checkCat(doCheck, line)(void *data) \
	{ \
		struct checkEventErrorsParams *p = (struct checkEventErrorsParams *) data; \
		(void) p; /* in the event call does not use this */ \
		call; \
	}
#define checkErrorCase(call, msgWant) checkErrorCaseFull(__LINE__, call, msgWant)
#include "controls_errors.h"
#undef checkErrorCaseFull
#undef checkErrorCase

static const struct {
	const char *name;
	void (*f)(void *data);
	const char *msgWant;
} controlErrorCases[] = {
#define checkErrorCaseFull(line, callstr, msgWant) { callstr, checkCat(doCheck, line), msgWant },
#define checkErrorCase(call, msgWant) checkErrorCaseFull(__LINE__, #call, msgWant)
#include "events_errors.h"
#undef checkErrorCase
#undef checkErrorCaseFull
#undef checkCat
	{ NULL, NULL, NULL, },
};

testingTest(ControlErrors)
{
	struct checkControlErrorsParams p;
	size_t i;

	memset(&p, 0, sizeof (struct checkControlErrorsParams));
	p.namePlaceholder = "name";
	createTestVtable(&p.vtablePlaceholder);
	// TODO osVtablePlaceholder
	p.implDataSizePlaceholder = sizeof (struct testImplData);
	p.badSizeVtable.Size = 1;

	for (i = 0; controlErrorCases[i].name != NULL; i++)
		checkProgrammerError(t, controlErrorCases[i].name, controlErrorCases[i].f, &p, controlErrorCases[i].msgWant);
}
