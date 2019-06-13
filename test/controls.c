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

uiControlVtable *allocVtableFull(testingT *t, const char *file, long line)
{
	uiControlVtable *v;

	v = (uiControlVtable *) malloc(sizeof (uiControlVtable));
	if (v == NULL)
		testingTFatalfFull(t, file, line, "memory exhausted allocating uiControlVtable");
	memset(v, 0, sizeof (uiControlVtable));
	v->Size = sizeof (uiControlVtable);
	v->Init = testVtableInit;
	v->Free = testVtableFree;
	return v;
}

struct checkControlErrorsParams {
	const char *namePlaceholder;
	uiControlVtable *vtablePlaceholder;
	uiControlOSVtable *osVtablePlaceholder;
	size_t implDataSizePlaceholder;
	uiControlVtable *vtableBadSize;
};

// TODO clean up these macros
#define checkCat(a, b) a ## b
#define checkErrorCaseFull(line, call, msgWant) \
	static void checkCat(doCheck, line)(void *data) \
	{ \
		struct checkControlErrorsParams *p = (struct checkControlErrorsParams *) data; \
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
#include "controls_errors.h"
#undef checkErrorCase
#undef checkErrorCaseFull
#undef checkCat
	{ NULL, NULL, NULL, },
};

testingTest(ControlErrors)
{
	struct checkControlErrorsParams p;
	uiControlVtable vtableBadSize;
	size_t i;

	memset(&p, 0, sizeof (struct checkControlErrorsParams));
	p.namePlaceholder = "name";
	p.vtablePlaceholder = allocVtable(t);
	testingTDefer(t, deferFree, p.vtablePlaceholder);
	// TODO osVtablePlaceholder
	p.implDataSizePlaceholder = sizeof (struct testImplData);
	memset(&vtableBadSize, 0, sizeof (uiControlVtable));
	vtableBadSize.Size = 1;
	p.vtableBadSize = &vtableBadSize;

	for (i = 0; controlErrorCases[i].name != NULL; i++)
		checkProgrammerError(t, controlErrorCases[i].name, controlErrorCases[i].f, &p, controlErrorCases[i].msgWant);
}
