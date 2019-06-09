// 10 april 2019
#include "test.h"

void timeoutMain(void *data)
{
	uiMain();
}

struct errorParams errorParams;

void catchProgrammerError(const char *prefix, const char *msg, const char *suffix, bool internal)
{
	errorParams.caught = true;
	if (strcmp(prefix, "libui programmer error") != 0)
		// TODO use diff
		testingTErrorfFull(errorParams.t, errorParams.file, errorParams.line, "%s prefix string doesn't contain \"programmer error\": %s", errorParams.exprstr, prefix);
	if (internal)
		testingTErrorfFull(errorParams.t, errorParams.file, errorParams.line, "%s error is marked internal; should not have been", errorParams.exprstr);
	if (strcmp(msg, errorParams.msgWant) != 0)
		// TODO use diff
		testingTErrorfFull(errorParams.t, errorParams.file, errorParams.line, "%s: message doesn't contain expected substring:" diff("%s"),
			errorParams.exprstr, msg, errorParams.msgWant);
}

static void runSetORingResults(testingSet *set, const struct testingOptions *options, bool *anyRun, bool *anyFailed)
{
	bool ar, af;

	testingSetRun(set, options, &ar, &af);
	if (ar)
		*anyRun = true;
	if (af)
		*anyFailed = true;
}

int main(int argc, char *argv[])
{
	testingOptions opts;
	bool anyRun = false, anyFailed = false;
	uiInitError err;

	memset(&opts, 0, sizeof (testingOptions));
	if (argc == 2 && strcmp(argv[1], "-v") == 0)
		opts.Verbose = true;
	else if (argc != 1) {
		fprintf(stderr, "usage: %s [-v]\n", argv[0]);
		return 1;
	}

	runSetORingResults(beforeTests, &opts, &anyRun, &anyFailed);
	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	if (!uiInit(NULL, &err)) {
		fprintf(stderr, "uiInit() failed: %s; can't continue\n", err.Message);
		printf("FAIL\n");
		return 1;
	}
	runSetORingResults(NULL, &opts, &anyRun, &anyFailed);

	if (!anyRun)
		fprintf(stderr, "warning: no tests to run\n");
	if (anyFailed) {
		printf("FAIL\n");
		return 1;
	}
	printf("PASS\n");
	fflush(stdout);		// AddressSanitizer can chop the tail end of the output for whatever reason
	return 0;
}
