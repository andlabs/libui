// 8 june 2019
#include "test.h"

struct counts {
	unsigned long countInit;
	unsigned long countFree;
};

static void checkCountsFull(testingT *t, const char *file, long line, const struct counts *got, const struct counts *want)
{
#define check(method) \
	if (got->count ## method != want->count ## method) \
		testingTErrorfFull(t, file, line, "wrong number of calls to " #method "():" diff("%lu"), \
			got->count ## method, want->count ## method)
	check(Init);
	check(Free);
#undef check
}

#define checkCounts(t, got, want) checkCountsFull(t, __FILE__, __LINE__, got, want)

struct testImplData {
	struct counts *counts;
};

static struct counts failInit;
void *testControlFailInit = &failInit;

// TODO document that impl data is zero-initialized before this is called
// TODO we'll also have to eventually deal with the fact that NULL is not required to be 0... or at least confirm that
static bool testVtableInit(uiControl *c, void *implData, void *initData)
{
	struct testImplData *d = (struct testImplData *) implData;
	struct counts *counts = (struct counts *) initData;

	if (initData == testControlFailInit)
		return false;
	if (initData == NULL)
		return true;
	if (d->counts == NULL)
		d->counts = counts;
	d->counts->countInit++;
	return true;
}

static void testVtableFree(uiControl *c, void *implData)
{
	struct testImplData *d = (struct testImplData *) implData;

	if (d->counts != NULL)
		d->counts->countFree++;
}

static const uiControlVtable vtable = {
	.Size = sizeof (uiControlVtable),
	.Init = testVtableInit,
	.Free = testVtableFree,
};

const uiControlVtable *testVtable(void)
{
	return &vtable;
}

size_t testImplDataSize(void)
{
	return sizeof (struct testImplData);
}

// TODO explicitly make/document 0 as always invalid
uint32_t testControlType = 0;
uint32_t testControlType2 = 0;

testingTest(ControlMethodsCalled)
{
	uiControl *c;
	struct counts counts;
	struct counts want;

	testingTLogf(t, "*** uiNewControl()");
	memset(&counts, 0, sizeof (struct counts));
	c = uiNewControl(testControlType, &counts);
	memset(&want, 0, sizeof (struct counts));
	want.countInit = 1;
	checkCounts(t, &counts, &want);

	testingTLogf(t, "*** uiControlFree()");
	memset(&counts, 0, sizeof (struct counts));
	// TODO add event handler
	uiControlFree(c);
	memset(&want, 0, sizeof (struct counts));
	want.countFree = 1;
	checkCounts(t, &counts, &want);
}

// TODO test freeing a parent frees the child
