// 4 december 2014
#include "uipriv_windows.hpp"

typedef std::vector<uint8_t> byteArray;

static std::map<uint8_t *, byteArray *> heap;
static std::map<byteArray *, const char *> types;

void initAlloc(void)
{
	// do nothing
}

void uninitAlloc(void)
{
	BOOL hasEntry;

	hasEntry = FALSE;
	for (const auto &alloc : heap) {
		if (!hasEntry) {
			fprintf(stderr, "[libui] leaked allocations:\n");
			hasEntry = TRUE;
		}
		fprintf(stderr, "[libui] %p %s\n",
			alloc.first,
			types[alloc.second]);
	}
	if (hasEntry)
		complain("either you left something around or there's a bug in libui");
}

#define rawBytes(pa) (&((*pa)[0]))

void *uiAlloc(size_t size, const char *type)
{
	byteArray *out;

	out = new byteArray(size, 0);
	heap[rawBytes(out)] = out;
	types[out] = type;
	return rawBytes(out);
}

void *uiRealloc(void *_p, size_t size, const char *type)
{
	uint8_t *p = (uint8_t *) _p;
	byteArray *arr;

	if (p == NULL)
		return uiAlloc(size, type);
	arr = heap[p];
	arr->resize(size, 0);
	heap.erase(p);
	heap[rawBytes(arr)] = arr;
	return rawBytes(arr);
}

void uiFree(void *_p)
{
	uint8_t *p = (uint8_t *) _p;

	if (p == NULL)
		implbug("attempt to uiFree(NULL)");
	types.erase(heap[p]);
	delete heap[p];
	heap.erase(p);
}
