// 28 may 2019
#include "test.h"

struct errorCase {
	const char *name;
	const char *file;
	long line;
	bool caught;
	char *prefixGot;
	bool internalGot;
	char *msgGot;
	const char *msgWant;
	struct errorCase *next;
};

static struct errorCase *current = NULL;
static char *caseError = NULL;
static char caseErrorMemoryExhausted[] = "memory exhausted";
static char caseErrorEncodingError[] = "encoding error while handling other case error";

#define sharedbitsPrefix priv
#define sharedbitsStatic static
#ifdef _WIN32
// While we do only need strncpy(), our privInternalError() calls vsnprintf(), so include that too.
#include "../../sharedbits/strsafe_impl.h"
static void privInternalError(const char *fmt, ...)
{
	va_list ap, ap2;
	int n;

	va_start(ap, fmt);
	va_copy(ap2, ap);
	n = privVsnprintf(NULL, 0, fmt, ap2);
	va_end(ap2);
	if (n < 0) {
		caseError = caseErrorEncodingError;
		va_end(ap);
		return;
	}
	caseError = (char *) malloc((n + 1) * sizeof (char));
	if (caseError == NULL) {
		caseError = caseErrorMemoryExhausted;
		va_end(ap);
		return;
	}
	privVsnprintf(caseError, n + 1, fmt, ap);
	va_end(ap);
}
#else
#include "../../sharedbits/strsafe_strncpy_impl.h"
#endif
#undef sharedbitsStatic
#undef sharedbitsPrefix

static void catalogProgrammerError(const char *prefix, const char *msg, const char *suffix, bool internal)
{
	size_t n;

	current->caught = true;
	if (strstr(prefix, "programmer error") == NULL) {
		n = strlen(prefix);
		current->prefixGot = (char *) malloc((n + 1) * sizeof (char));
		if (current->prefixGot == NULL) {
			caseError = caseErrorMemoryExhausted;
			return;
		}
		privStrncpy(current->prefixGot, prefix, n + 1);
		if (caseError != NULL)
			return;
	}
	current->internalGot = internal;
	if (strstr(msg, current->msgWant) == NULL) {
		n = strlen(msg);
		current->msgGot = (char *) malloc((n + 1) * sizeof (char));
		if (current->msgGot == NULL) {
			caseError = caseErrorMemoryExhausted;
			return;
		}
		privStrncpy(current->msgGot, msg, n + 1);
		if (caseError != NULL)
			return;
	}
}

static struct errorCase *newCase(void)
{
	struct errorCase *p;

	p = (struct errorCase *) malloc(sizeof (struct errorCase));
	if (p == NULL) {
		caseError = caseErrorMemoryExhausted;
		return NULL;
	}
	memset(p, 0, sizeof (struct errorCase));
	return p;
}

static void freeCases(struct errorCase *first)
{
	struct errorCase *p, *next;

	p = first;
	while (p != NULL) {
		if (p->prefixGot != NULL)
			free(p->prefixGot);
		if (p->msgGot != NULL)
			free(p->msgGot);
		next = p->next;
		free(p);
		p = next;
	}
}

static void reportCases(testingT *t, struct errorCase *p)
{
	while (p != NULL) {
		testingTLogfFull(t, p->file, p->line, "*** %s", p->name);
		if (!p->caught) {
			testingTErrorfFull(t, p->file, p->line, "%s did not throw a programmer error; should have", p->name);
			p = p->next;
			continue;
		}
		if (p->prefixGot != NULL)
			testingTErrorfFull(t, p->file, p->line, "%s prefix string doesn't contain \"programmer error\": %s", p->name, p->prefixGot);
		if (p->internalGot)
			testingTErrorfFull(t, p->file, p->line, "%s error is marked internal; should not have been", p->name);
		if (p->msgGot != NULL)
			testingTErrorfFull(t, p->file, p->line, "%s message doesn't contain expected substring:" diffx("%s"),
				p->name, p->msgGot, p->msgWant);
		p = p->next;
	}
}

#define allcallsCase(f, ...) { \
	current = newCase(); \
	if (caseError != NULL) \
		return first; \
	current->name = #f "()"; \
	current->file = __FILE__; \
	current->line = __LINE__; \
	current->msgWant = "attempt to call " #f "() " allcallsMsgSuffix; \
	f(__VA_ARGS__); \
	if (first == NULL) \
		first = current; \
	if (last != NULL) \
		last->next = current; \
	last = current; \
	if (caseError != NULL) \
		return first; \
}

static struct errorCase *runCasesBeforeInit(void)
{
	struct errorCase *first = NULL;
	struct errorCase *last = NULL;

#define allcallsMsgSuffix "before uiInit()"
	allcallsCase(uiQueueMain, NULL, NULL);
#include "allcalls.h"
#undef allcallsMsgSuffix
	return first;
}

testingTestInSet(beforeTests, FunctionsFailBeforeInit)
{
	struct errorCase *cases;

	caseError = NULL;
	uiprivTestHookReportProgrammerError(catalogProgrammerError);
	cases = runCasesBeforeInit();
	uiprivTestHookReportProgrammerError(NULL);
	if (caseError != NULL) {
		freeCases(cases);
		testingTErrorf(t, "%s running tests", caseError);
		if (caseError != caseErrorMemoryExhausted && caseError != caseErrorEncodingError)
			free(caseError);
		caseError = NULL;
		testingTFailNow(t);
	}
	reportCases(t, cases);
	freeCases(cases);
}

static struct errorCase *runCasesWrongThread(void)
{
	struct errorCase *first = NULL;
	struct errorCase *last = NULL;

#define allcallsMsgSuffix "on a thread other than the GUI thread"
#include "allcalls.h"
#undef allcallsMsgSuffix
	return first;
}

static void wrongThreadThreadProc(void *data)
{
	struct errorCase **pCases = (struct errorCase **) data;

	uiprivTestHookReportProgrammerError(catalogProgrammerError);
	*pCases = runCasesWrongThread();
	uiprivTestHookReportProgrammerError(NULL);
	// do this now in case memory was exhausted and something gets allocated before we return to the main thread
	if (caseError != NULL) {
		freeCases(*pCases);
		*pCases = NULL;
	}
}

testingTest(FunctionsFailOnWrongThread)
{
	struct errorCase *cases;
	threadThread *thread;
	threadSysError err;

	caseError = NULL;
	err = threadNewThread(wrongThreadThreadProc, &cases, &thread);
	if (err != 0)
		testingTFatalf(t, "error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	err = threadThreadWaitAndFree(thread);
	if (err != 0)
		testingTFatalf(t, "error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	if (caseError != NULL) {
		freeCases(cases);
		testingTErrorf(t, "%s running tests", caseError);
		if (caseError != caseErrorMemoryExhausted && caseError != caseErrorEncodingError)
			free(caseError);
		caseError = NULL;
		testingTFailNow(t);
	}
	reportCases(t, cases);
	freeCases(cases);
}
