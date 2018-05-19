// 17 august 2015
#import "uipriv_darwin.h"

// unfortunately NSMutableDictionary copies its keys, meaning we can't use it for pointers
// hence, this file
// we could expose a NSMapTable directly, but let's treat all pointers as opaque and hide the implementation, just to be safe and prevent even more rewrites later
struct uiprivMap {
	NSMapTable *m;
};

uiprivMap *uiprivNewMap(void)
{
	uiprivMap *m;

	m = uiprivNew(uiprivMap);
	m->m = [[NSMapTable alloc] initWithKeyOptions:(NSPointerFunctionsOpaqueMemory | NSPointerFunctionsOpaquePersonality)
		valueOptions:(NSPointerFunctionsOpaqueMemory | NSPointerFunctionsOpaquePersonality)
		capacity:0];
	return m;
}

void uiprivMapDestroy(uiprivMap *m)
{
	if ([m->m count] != 0)
		uiprivImplBug("attempt to destroy map with items inside");
	[m->m release];
	uiprivFree(m);
}

void *uiprivMapGet(uiprivMap *m, void *key)
{
	return NSMapGet(m->m, key);
}

void uiprivMapSet(uiprivMap *m, void *key, void *value)
{
	NSMapInsert(m->m, key, value);
}

void uiprivMapDelete(uiprivMap *m, void *key)
{
	NSMapRemove(m->m, key);
}

void uiprivMapWalk(uiprivMap *m, void (*f)(void *key, void *value))
{
	NSMapEnumerator e;
	void *k, *v;

	e = NSEnumerateMapTable(m->m);
	k = NULL;
	v = NULL;
	while (NSNextMapEnumeratorPair(&e, &k, &v))
		f(k, v);
	NSEndMapTableEnumeration(&e);
}

void uiprivMapReset(uiprivMap *m)
{
	NSResetMapTable(m->m);
}
