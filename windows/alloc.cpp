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

void *uiAlloc(size_t size, const char *type)
{
	byteArray *out;

	out = new byteArray(size, 0);
	heap[&out[0]] = out;
	types[out] = type;
	return &out[0];
}

void *uiRealloc(void *p, size_t size, const char *type)
{
	byteArray *arr;

	if (p == NULL)
		return uiAlloc(size, type);
	arr = heap[p];
	arr->resize(size, 0);
	heap.erase(p);
	heap[&arr[0]] = arr;
	return &arr[0];
}

void uiFree(void *p)
{
	if (p == NULL)
		complain("attempt to uiFree(NULL); there's a bug somewhere");
	types.erase(heap[p]);
	delete heap[p];
	heap.erase(p);
}
