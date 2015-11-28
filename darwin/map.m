// 17 august 2015
#import "uipriv_darwin.h"

// TODO are the NSValues being garbage collected underfoot? open menuless window and check the checkbox

// unfortunately NSMutableDictionary copies its keys, meaning we can't use it for pointers
// hence, this file

NSMapTable *newMap(void)
{
	return [NSMapTable mapTableWithKeyOptions:(NSPointerFunctionsOpaqueMemory | NSPointerFunctionsOpaquePersonality)
		valueOptions:NSPointerFunctionsOpaqueMemory];
}

void *mapGet(NSMapTable *map, id key)
{
	NSValue *v;

	v = (NSValue *) [map objectForKey:key];
	return [v pointerValue];
}

void mapSet(NSMapTable *map, id key, void *value)
{
	NSValue *v;

	v = [NSValue valueWithPointer:value];
	[map setObject:v forKey:key];
}
