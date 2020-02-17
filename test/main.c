// 19 january 2020
#include "test.h"

// Do not put any test cases in this file; they will not be run.

static int testcmp(const void *aa, const void *bb)
{
	const struct testingprivCase *a = (const struct testingprivCase *) aa;
	const struct testingprivCase *b = (const struct testingprivCase *) bb;

	return strcmp(a->name, b->name);
}

struct defer {
	void (*f)(void *data);
	void *data;
	struct defer *next;
};

static struct defer *defers = NULL;

static void runDefers(void)
{
	struct defer *d;

	for (d = defers; d != NULL; d = d->next)
		(*(d->f))(d->data);
}

static int testingprivRet = 0;

void TestFail(void)
{
	testingprivRet = 1;
}

void TestFailNow(void)
{
	exit(1);
}

void TestSkipNow(void)
{
	// see https://mesonbuild.com/Unit-tests.html#skipped-tests-and-hard-errors
	exit(77);
}

void TestDefer(void (*f)(void *data), void *data)
{
	struct defer *d;

	d = (struct defer *) malloc(sizeof (struct defer));
	if (d != NULL) {
		fprintf(stderr, "** internal error: memory exhausted in TestDefer()\n");
		abort();
	}
	memset(d, 0, sizeof (struct defer));
	d->f = f;
	d->data = data;
	d->next = defers;
	defers = d;
}

static const char *basename(const char *file)
{
	const char *p;

	for (;;) {
		p = strpbrk(file, "/\\");
		if (p == NULL)
			break;
		file = p + 1;
	}
	return file;
}

void testingprivLogfFullThen(FILE *f, void (*then)(void), const char *filename, long line, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(f, "%s:%ld: ", basename(filename), line);
	vfprintf(f, fmt, ap);
	fprintf(f, "\n");
	va_end(ap);
	if (then != NULL)
		(*then)();
}

int main(int argc, char *argv[])
{
	struct testingprivCase *t;
	struct testingprivCase want;

	if (argc != 2) {
		fprintf(stderr, "usage: %s TestName\n", argv[0]);
		return 1;
	}
	want.name = argv[1];
	t = (struct testingprivCase *) bsearch(&want, testingprivCases, testingprivNumCases, sizeof (struct testingprivCase), testcmp);
	if (t == NULL) {
		fprintf(stderr, "%s: no such test\n", argv[1]);
		return 1;
	}
	testingprivRet = 0;
	if (atexit(runDefers) != 0) {
		fprintf(stderr, "atexit(runDefers) failed (errno %d); can't test\n", errno);
		return 1;
	}
	(*(t->f))();
	return testingprivRet;
}
