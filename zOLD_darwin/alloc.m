// 4 december 2014
#import <stdlib.h>
#import "uipriv_darwin.h"

static NSMutableArray *allocations;
NSMutableArray *uiprivDelegates;

void uiprivInitAlloc(void)
{
	allocations = [NSMutableArray new];
	uiprivDelegates = [NSMutableArray new];
}

#define UINT8(p) ((uint8_t *) (p))
#define PVOID(p) ((void *) (p))
#define EXTRA (sizeof (size_t) + sizeof (const char **))
#define DATA(p) PVOID(UINT8(p) + EXTRA)
#define BASE(p) PVOID(UINT8(p) - EXTRA)
#define SIZE(p) ((size_t *) (p))
#define CCHAR(p) ((const char **) (p))
#define TYPE(p) CCHAR(UINT8(p) + sizeof (size_t))

void uiprivUninitAlloc(void)
{
	NSMutableString *str;
	NSValue *v;

	[uiprivDelegates release];
	if ([allocations count] == 0) {
		[allocations release];
		return;
	}
	str = [NSMutableString new];
	for (v in allocations) {
		void *ptr;

		ptr = [v pointerValue];
		[str appendString:[NSString stringWithFormat:@"%p %s\n", ptr, *TYPE(ptr)]];
	}
	uiprivUserBug("Some data was leaked; either you left a uiControl lying around or there's a bug in libui itself. Leaked data:\n%s", [str UTF8String]);
	[str release];
}

void *uiprivAlloc(size_t size, const char *type)
{
	void *out;

	out = malloc(EXTRA + size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiAlloc()\n");
		abort();
	}
	memset(DATA(out), 0, size);
	*SIZE(out) = size;
	*TYPE(out) = type;
	[allocations addObject:[NSValue valueWithPointer:out]];
	return DATA(out);
}

void *uiprivRealloc(void *p, size_t new, const char *type)
{
	void *out;
	size_t *s;

	if (p == NULL)
		return uiprivAlloc(new, type);
	p = BASE(p);
	out = realloc(p, EXTRA + new);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiprivRealloc()\n");
		abort();
	}
	s = SIZE(out);
	if (new > *s)
		memset(((uint8_t *) DATA(out)) + *s, 0, new - *s);
	*s = new;
	[allocations removeObject:[NSValue valueWithPointer:p]];
	[allocations addObject:[NSValue valueWithPointer:out]];
	return DATA(out);
}

void uiprivFree(void *p)
{
	if (p == NULL)
		uiprivImplBug("attempt to uiprivFree(NULL)");
	p = BASE(p);
	free(p);
	[allocations removeObject:[NSValue valueWithPointer:p]];
}
