// 4 december 2014
#include "uipriv_windows.h"

// wrappers for allocator of choice
// panics on memory exhausted, undefined on heap corruption or other unreliably-detected malady (see http://stackoverflow.com/questions/28761680/is-there-a-windows-api-memory-allocator-deallocator-i-can-use-that-will-just-giv)
// new memory is set to zero
// passing NULL to tableRealloc() acts like tableAlloc()
// passing NULL to tableFree() is a no-op

static HANDLE heap;

int initAlloc(void)
{
	heap = HeapCreate(0, 0, 0);
	return heap != NULL;
}

#define UINT8(p) ((uint8_t *) (p))
#define PVOID(p) ((void *) (p))
#define EXTRA (sizeof (const char **))
#define DATA(p) PVOID(UINT8(p) + EXTRA)
#define BASE(p) PVOID(UINT8(p) - EXTRA)
#define CCHAR(p) ((const char **) (p))
#define TYPE(p) CCHAR(UINT8(p))

void uninitAlloc(void)
{
	BOOL hasEntry;
	PROCESS_HEAP_ENTRY phe;
	DWORD le;

	hasEntry = FALSE;
	ZeroMemory(&phe, sizeof (PROCESS_HEAP_ENTRY));
	while (HeapWalk(heap, &phe) != 0) {
		// skip non-allocations
		if ((phe.wFlags & PROCESS_HEAP_ENTRY_BUSY) == 0)
			continue;
		if (!hasEntry) {
			fprintf(stderr, "[libui] leaked allocations:\n");
			hasEntry = TRUE;
		}
		fprintf(stderr, "[libui] %p %s\n", phe.lpData, *TYPE(phe.lpData));
	}
	le = GetLastError();
	SetLastError(le);		// just in case
	if (le != ERROR_NO_MORE_ITEMS)
		logLastError("error walking heap in uninitAlloc()");
	if (hasEntry)
		complain("either you left something around or there's a bug in libui");
	if (HeapDestroy(heap) == 0)
		logLastError("error destroying heap in uninitAlloc()");
}

void *uiAlloc(size_t size, const char *type)
{
	void *out;

	out = HeapAlloc(heap, HEAP_ZERO_MEMORY, EXTRA + size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiAlloc()\n");
		abort();
	}
	*TYPE(out) = type;
	return DATA(out);
}

void *uiRealloc(void *p, size_t size, const char *type)
{
	void *out;

	if (p == NULL)
		return uiAlloc(size, type);
	p = BASE(p);
	out = HeapReAlloc(heap, HEAP_ZERO_MEMORY, p, EXTRA + size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiRealloc()\n");
		abort();
	}
	return DATA(out);
}

void uiFree(void *p)
{
	if (p == NULL)
		complain("attempt to uiFree(NULL); there's a bug somewhere");
	p = BASE(p);
	if (HeapFree(heap, 0, p) == 0)
		logLastError("error freeing memory in uiFree()");
}
