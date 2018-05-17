// 17 may 2018
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define WINVER			0x0600	/* from Microsoft's winnls.h */
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600	/* from Microsoft's pdh.h */
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include <psapi.h>
#include <stdio.h>

int affinityCount(DWORD_PTR a)
{
	int n = 0;

	while (a != 0) {
		if ((a & 1) != 0)
			n++;
		a >>= 1;
	}
	return n;
}

int main(void)
{
	HANDLE pseudoCurrent, current;
	DWORD_PTR pa, sa;
	SYSTEM_INFO si;
	DWORD le;

	pseudoCurrent = GetCurrentProcess();
	if (GetProcessAffinityMask(pseudoCurrent, &pa, &sa) != 0) {
		printf("GetProcessAffinityMask(GetCurrentProcess()):\n");
		printf("\tProcess - %d\n", affinityCount(pa));
		printf("\tSystem  - %d\n", affinityCount(sa));
	} else {
		le = GetLastError();
		fprintf(stderr, "error calling GetProcessAffinity(GetCurrentProcess()): %I32d\n", le);
	}

	if (DuplicateHandle(pseudoCurrent, pseudoCurrent,
		pseudoCurrent, &current,
		0, FALSE, DUPLICATE_SAME_ACCESS) != 0)
		if (GetProcessAffinityMask(current, &pa, &sa) != 0) {
			printf("GetProcessAffinityMask(real handle):\n");
			printf("\tProcess - %d\n", affinityCount(pa));
			printf("\tSystem  - %d\n", affinityCount(sa));
		} else {
			le = GetLastError();
			fprintf(stderr, "error calling GetProcessAffinity(real handle): %I32d\n", le);
		}
	else {
		le = GetLastError();
		fprintf(stderr, "error calling DuplicateHandle(GetCurrentProcess()) (no real handle info available): %I32d\n", le);
	}

	ZeroMemory(&si, sizeof (SYSTEM_INFO));
	GetSystemInfo(&si);
	printf("GetSystemInfo() processor count: %I32d\n", si.dwNumberOfProcessors);
	ZeroMemory(&si, sizeof (SYSTEM_INFO));
	GetNativeSystemInfo(&si);
	printf("GetNativeSystemInfo() processor count: %I32d\n", si.dwNumberOfProcessors);

	return 0;
}
