// 27 february 2018
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include "testing.h"

#define testingprivNew(T) ((T *) malloc(sizeof (T)))

struct defer {
	void (*f)(testingT *, void *);
	void *data;
	struct defer *next;
};

#ifdef _MSC_VER
// Microsoft defines jmp_buf with a __declspec(align()), and for whatever reason, they have a warning that triggers when you use that for any reason, and that warning is enabled with /W4
// Silence the warning; it's harmless.
#pragma warning(push)
#pragma warning(disable: 4324)
#endif

struct testingT {
	const char *name;
	void (*f)(testingT *);
	const char *file;
	long line;
	int failed;
	int skipped;
	int returned;
	jmp_buf returnNowBuf;
	struct defer *defers;
	int defersRun;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static void initTest(testingT *t, const char *name, void (*f)(testingT *), const char *file, long line)
{
	t->name = name;
	t->f = f;
	t->file = file;
	t->line = line;
	t->failed = 0;
	t->skipped = 0;
	t->returned = 0;
	t->defers = NULL;
	t->defersRun = 0;
}

#define nGrow 32

struct testset {
	testingT *tests;
	size_t len;
	size_t cap;
};

static struct testset tests = { NULL, 0, 0 };
static struct testset testsBefore = { NULL, 0, 0 };
static struct testset testsAfter = { NULL, 0, 0 };

static void testsetAdd(struct testset *set, const char *name, void (*f)(testingT *), const char *file, long line)
{
	if (set->len == set->cap) {
		testingT *newbuf;

		set->cap += nGrow;
		newbuf = (testingT *) realloc(set->tests, set->cap * sizeof (testingT));
		// TODO abort if newbuf is NULL
		set->tests = newbuf;
	}
	initTest(set->tests + set->len, name, f, file, line);
	set->len++;
}

void testingprivRegisterTest(const char *name, void (*f)(testingT *), const char *file, long line)
{
	testsetAdd(&tests, name, f, file, line);
}

void testingprivRegisterTestBefore(const char *name, void (*f)(testingT *), const char *file, long line)
{
	testsetAdd(&testsBefore, name, f, file, line);
}

void testingprivRegisterTestAfter(const char *name, void (*f)(testingT *), const char *file, long line)
{
	testsetAdd(&testsAfter, name, f, file, line);
}

static int testcmp(const void *a, const void *b)
{
	const testingT *ta = (const testingT *) a;
	const testingT *tb = (const testingT *) b;
	int ret;

	ret = strcmp(ta->file, tb->file);
	if (ret != 0)
		return ret;
	if (ta->line < tb->line)
		return -1;
	if (ta->line > tb->line)
		return 1;
	return 0;
}

static void testsetSort(struct testset *set)
{
	qsort(set->tests, set->len, sizeof (testingT), testcmp);
}

static void runDefers(testingT *t)
{
	struct defer *d;

	if (t->defersRun)
		return;
	t->defersRun = 1;
	for (d = t->defers; d != NULL; d = d->next)
		(*(d->f))(t, d->data);
}

static void testsetRun(struct testset *set, int *anyFailed)
{
	size_t i;
	testingT *t;
	const char *status;
	testingTimer *timer;
	char *timerstr;

	t = set->tests;
	timer = testingNewTimer();
	for (i = 0; i < set->len; i++) {
		printf("=== RUN   %s\n", t->name);
		testingTimerStart(timer);
		if (setjmp(t->returnNowBuf) == 0)
			(*(t->f))(t);
		testingTimerEnd(timer);
		t->returned = 1;
		runDefers(t);
		status = "PASS";
		if (t->failed) {
			status = "FAIL";
			*anyFailed = 1;
		} else if (t->skipped)
			// note that failed overrides skipped
			status = "SKIP";
		timerstr = testingTimerString(timer);
		printf("--- %s: %s (%s)\n", status, t->name, timerstr);
		testingFreeTimerString(timerstr);
		t++;
	}
	testingFreeTimer(timer);
}

int testingMain(void)
{
	int anyFailed;

	// TODO see if this should run if all tests are skipped
	if ((testsBefore.len + tests.len + testsAfter.len) == 0) {
		fprintf(stderr, "warning: no tests to run\n");
		// imitate Go here (TODO confirm this)
		return 0;
	}

	testsetSort(&testsBefore);
	testsetSort(&tests);
	testsetSort(&testsAfter);

	anyFailed = 0;
	testsetRun(&testsBefore, &anyFailed);
	// TODO print a warning that we skip the next stages if a prior stage failed?
	if (!anyFailed)
		testsetRun(&tests, &anyFailed);
	// TODO should we unconditionally run these tests if before succeeded but the main tests failed?
	if (!anyFailed)
		testsetRun(&testsAfter, &anyFailed);
	if (anyFailed) {
		printf("FAIL\n");
		return 1;
	}
	printf("PASS\n");
	return 0;
}

void testingprivTLogfFull(testingT *t, const char *file, long line, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	testingprivTLogvfFull(t, file, line, format, ap);
	va_end(ap);
}

void testingprivTLogvfFull(testingT *t, const char *file, long line, const char *format, va_list ap)
{
	// TODO extract filename from file
	printf("\t%s:%ld: ", file, line);
	// TODO split into lines separated by \n\t\t and trimming trailing empty lines
	vprintf(format, ap);
	printf("\n");
}

void testingTFail(testingT *t)
{
	t->failed = 1;
}

static void returnNow(testingT *t)
{
	if (!t->returned) {
		// set this now so a FailNow inside a Defer doesn't longjmp twice
		t->returned = 1;
		// run defers before calling longjmp() just to be safe
		runDefers(t);
		longjmp(t->returnNowBuf, 1);
	}
}

void testingprivTFailNow(testingT *t)
{
	testingTFail(t);
	returnNow(t);
}

void testingprivTSkipNow(testingT *t)
{
	t->skipped = 1;
	returnNow(t);
}

void testingTDefer(testingT *t, void (*f)(testingT *t, void *data), void *data)
{
	struct defer *d;

	d = testingprivNew(struct defer);
	d->f = f;
	d->data = data;
	// add to the head of the list so defers are run in reverse order of how they were added
	d->next = t->defers;
	t->defers = d;
}

// This is based on the algorithm that Go uses for time.Duration.
// Of course, we're not expressing it the same way...
struct timerStringPart {
	char suffix;
	char suffix2;
	int mode;
	uint32_t maxOrMod;
	int precision;
};

enum {
	modeMaxAndStop,
	modeFracModContinue,
};

static const struct timerStringPart parts[] = {
	{ 'n', 's', modeMaxAndStop, 1000, 0 },
	{ 'u', 's', modeMaxAndStop, 1000000, 3 },
	{ 'm', 's', modeMaxAndStop, 1000000000, 6 },
	{ 's', 0, modeFracModContinue, 60, 9 },
	{ 'm', 0, modeFracModContinue, 60, 0 },
	{ 'h', 0, modeFracModContinue, 60, 0 },
	{ 0, 0, 0, 0, 0 },
};

static void fillFracPart(char *s, int precision, int *start, uint64_t *unsec)
{
	int i;
	int print;
	uint64_t digit;

	print = 0;
	for (i = 0; i < precision; i++) {
		digit = *unsec % 10;
		print = print || (digit == 0);
		if (print) {
			s[*start - 1] = "0123456789"[digit];
			(*start)--;
		}
		*unsec /= 10;
	}
	if (print) {
		s[*start - 1] = '.';
		(*start)--;
	}
}

static void fillIntPart(char *s, int *start, uint64_t unsec)
{
	if (unsec == 0) {
		s[*start - 1] = '0';
		(*start)--;
		return;
	}
	while (unsec != 0) {
		s[*start - 1] = "0123456789"[unsec % 10];
		(*start)--;
		unsec /= 10;
	}
}

char *testingTimerString(testingTimer *t)
{
	int64_t nsec;
	uint64_t unsec;
	int neg;
	char *s;
	int start;
	const struct timerStringPart *p;

	// The Go algorithm says 32 should be enough.
	s = (char *) malloc(33 * sizeof (char));
	// TODO handle failure
	memset(s, 0, 33 * sizeof (char));
	start = 32;

	nsec = testingTimerNsec(t);
	if (nsec == 0) {
		s[0] = '0';
		s[1] = 's';
		return s;
	}
	unsec = (uint64_t) nsec;
	neg = 0;
	if (nsec < 0) {
		unsec = -unsec;
		neg = 1;
	}

	for (p = parts; p->suffix != 0; p++) {
		if (p->mode == modeMaxAndStop && unsec < p->maxOrMod) {
			if (p->suffix2 != 0) {
				s[start - 1] = p->suffix2;
				start--;
			}
			s[start - 1] = p->suffix;
			start--;
			fillFracPart(s, p->precision, &start, &unsec);
			fillIntPart(s, &start, unsec);
			break;
		}
		if (p->mode == modeFracModContinue && unsec != 0) {
			if (p->suffix2 != 0) {
				s[start - 1] = p->suffix2;
				start--;
			}
			s[start - 1] = p->suffix;
			start--;
			fillFracPart(s, p->precision, &start, &unsec);
			fillIntPart(s, &start, unsec % p->maxOrMod);
			unsec /= p->maxOrMod;
			// and move on to the next one
		}
	}

	if (neg) {
		s[start - 1] = '-';
		start--;
	}
	memmove(s, s + start, 33 - start);
	return s;
}

void testingFreeTimerString(char *s)
{
	free(s);
}
