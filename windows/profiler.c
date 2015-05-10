// 10 may 2015
#include "uipriv_windows.h"

static FILE *fprof = NULL;

__attribute__((no_instrument_function)) static void init(void)
{
	if (fprof != NULL)
		return;
	fprof = fopen("profiler.out", "w");
	if (fprof == NULL) {
		fprintf(stderr, "error opening profiler output file\n");
		abort();
	}
}

__attribute__((no_instrument_function)) void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	LARGE_INTEGER counter;

	init();
	QueryPerformanceCounter(&counter);
	fprintf(fprof, "enter %p %I64d\n", this_fn, counter.QuadPart);
	fflush(fprof);
}

__attribute__((no_instrument_function)) void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
	LARGE_INTEGER counter;

	init();
	QueryPerformanceCounter(&counter);
	fprintf(fprof, "leave %p %I64d\n", this_fn, counter.QuadPart);
	fflush(fprof);
}
