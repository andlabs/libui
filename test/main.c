// 10 april 2019
#include <stdio.h>
#include <string.h>
#include "test.h"

void timeoutMain(void *data)
{
	uiMain();
}

static void runSetORingResults(testingSet *set, const struct testingOptions *options, int *anyRun, int *anyFailed)
{
	int ar, af;

	testingSetRun(set, options, &ar, &af);
	if (ar)
		*anyRun = 1;
	if (af)
		*anyFailed = 1;
}

int main(int argc, char *argv[])
{
	testingOptions opts;
	int anyRun = 0, anyFailed = 0;
	uiInitError err;
	int ret;

	memset(&opts, 0, sizeof (testingOptions));
	if (argc == 2 && strcmp(argv[1], "-v") == 0)
		opts.Verbose = 1;
	else if (argc != 1) {
		fprintf(stderr, "usage: %s [-v]\n", argv[0]);
		return 1;
	}

	runSetORingResults(beforeTests, &opts, &anyRun, &anyFailed);
	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	ret = uiInit(NULL, &err);
	if (ret == 0) {
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
	return 0;
}
