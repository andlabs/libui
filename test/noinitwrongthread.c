// 28 may 2019
// TODO get rid of the need for this (it temporarily silences noise so I can find actual build issues)
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdlib.h>
#include <string.h>
#include "lib/thread.h"
#include "test.h"

struct errorCase {
	const char *name;
	bool caught;
	char *prefixGot;
	bool internalGot;
	char *msgGot;
	const char *msgWant;
	struct errorCase *next;
};

static struct errorCase *current = NULL;
static bool memoryExhausted = false;

static void catalogProgrammerError(const char *prefix, const char *msg, const char *suffix, bool internal)
{
	current->caught = true;
	if (strstr(prefix, "programmer error") == NULL) {
		current->prefixGot = (char *) malloc((strlen(prefix) + 1) * sizeof (char));
		if (current->prefixGot == NULL) {
			memoryExhausted = true;
			return;
		}
		strcpy(current->prefixGot, prefix);
	}
	current->internalGot = internal;
	if (strstr(msg, current->msgWant) == NULL) {
		current->msgGot = (char *) malloc((strlen(msg) + 1) * sizeof (char));
		if (current->msgGot == NULL) {
			memoryExhausted = true;
			return;
		}
		strcpy(current->msgGot, msg);
	}
}

static struct errorCase *newCase(void)
{
	struct errorCase *p;

	p = (struct errorCase *) malloc(sizeof (struct errorCase));
	if (p == NULL) {
		memoryExhausted = true;
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
		testingTLogf(t, "*** %s", p->name);
		if (!p->caught) {
			testingTErrorf(t, "%s did not throw a programmer error; should have", p->name);
			p = p->next;
			continue;
		}
		if (p->prefixGot != NULL)
			testingTErrorf(t, "%s prefix string doesn't contain \"programmer error\": %s", p->name, p->prefixGot);
		if (p->internalGot)
			testingTErrorf(t, "%s error is marked internal; should not have been", p->name);
		if (p->msgGot != NULL)
			diff_2str(t, p->name, "message doesn't contain expected substring",
				"%s", p->msgGot, p->msgWant);
		p = p->next;
	}
}

#define allcallsCase(f, ...) { \
	current = newCase(); \
	if (memoryExhausted) \
		return first; \
	current->name = #f "()"; \
	current->msgWant = "attempt to call " #f "() " allcallsMsgSuffix; \
	f(__VA_ARGS__); \
	if (first == NULL) \
		first = current; \
	if (last != NULL) \
		last->next = current; \
	last = current; \
	if (memoryExhausted) \
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

	memoryExhausted = false;
	uiprivTestHookReportProgrammerError(catalogProgrammerError);
	cases = runCasesBeforeInit();
	uiprivTestHookReportProgrammerError(NULL);
	if (memoryExhausted) {
		freeCases(cases);
		testingTFatalf(t, "memory exhausted running tests");
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
	// do this now in case something gets allocated before we return to the main thread
	if (memoryExhausted) {
		freeCases(*pCases);
		*pCases = NULL;
	}
}

testingTest(FunctionsFailOnWrongThread)
{
	struct errorCase *cases;
	threadThread *thread;
	threadSysError err;

	memoryExhausted = false;
	err = threadNewThread(wrongThreadThreadProc, &cases, &thread);
	if (err != 0)
		testingTFatalf(t, "error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	err = threadThreadWaitAndFree(thread);
	if (err != 0)
		testingTFatalf(t, "error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	if (memoryExhausted)
		testingTFatalf(t, "memory exhausted running tests");
	reportCases(t, cases);
	freeCases(cases);
}
