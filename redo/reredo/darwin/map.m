// 17 august 2015
#import "uipriv_darwin.h"

// unfortunately NSMutableDictionary copies its keys, meaning we can't use it for pointers
// hence, this file

NSMapTable *newMap(void)
{
	// TODO NSPointerFunctionsOpaquePersonality?
	return [NSMapTable mapTableWithKeyOptions:NSPointerFunctionsOpaqueMemory
		valueOptions:NSPointerFunctionsOpaqueMemory];
}

void *mapGet(NSMapTable *map, id key)
{
	NSValue *v;

	v = (NSValue *) [self->items objectForKey:sender];
	return [v pointerValue];
}

void mapSet(NSMapTable *map, id key, void *value)
{
	NSValue *v;

	v = [NSValue valueWithPointer:value];
	[map setObject:v forKey:key];
}
