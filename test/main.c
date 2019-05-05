// 10 april 2019
#include <stdio.h>
#include <string.h>
#include "test.h"

void timeoutMain(void *data)
{
	uiMain();
}

int main(int argc, char *argv[])
{
	testingOptions opts;

	memset(&opts, 0, sizeof (testingOptions));
	if (argc == 2 && strcmp(argv[1], "-v") == 0)
		opts.Verbose = 1;
	else if (argc != 1) {
		fprintf(stderr, "usage: %s [-v]\n", argv[0]);
		return 1;
	}
	return testingMain(&opts);
}
