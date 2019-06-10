// 10 april 2019
#include "test.h"

void timeoutMain(void *data)
{
	uiMain();
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
