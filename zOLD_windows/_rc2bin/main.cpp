// 2 may 2018
#include "winapi.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "resources.hpp"

// TODO make sure there are no CRs in the output

void die(const char *f, const char *constname)
{
	DWORD le;

	le = GetLastError();
	fprintf(stderr, "error calling %s for %s: %I32d\n", f, constname, le);
	exit(1);
}

void dumpResource(const char *constname, const WCHAR *name, const WCHAR *type)
{
	HRSRC hrsrc;
	HGLOBAL res;
	uint8_t *b, *bp;
	DWORD i, n;
	DWORD j;

	hrsrc = FindResourceW(NULL, name, type);
	if (hrsrc == NULL)
		die("FindResourceW()", constname);
	n = SizeofResource(NULL, hrsrc);
	if (n == 0)
		die("SizeofResource()", constname);
	res = LoadResource(NULL, hrsrc);
	if (res == NULL)
		die("LoadResource()", constname);
	b = (uint8_t *) LockResource(res);
	if (b == NULL)
		die("LockResource()", constname);

	printf("static const uint8_t %s[] = {\n", constname);
	bp = b;
	j = 0;
	for (i = 0; i < n; i++) {
		if (j == 0)
			printf("\t");
		printf("0x%02I32X,", (uint32_t) (*bp));
		bp++;
		if (j == 7) {
			printf("\n");
			j = 0;
		} else {
			printf(" ");
			j++;
		}
	}
	if (j != 0)
		printf("\n");
	printf("};\n");
	printf("static_assert(ARRAYSIZE(%s) == %I32d, \"wrong size for resource %s\");\n", constname, n, constname);
	printf("\n");
}

int main(void)
{
#define d(c, t) dumpResource(#c, MAKEINTRESOURCEW(c), t)
	d(rcTabPageDialog, RT_DIALOG);
	d(rcFontDialog, RT_DIALOG);
	d(rcColorDialog, RT_DIALOG);
	return 0;
}
