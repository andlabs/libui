// 7 april 2015
#import "uipriv_darwin.h"

// also fine for NSCells and NSTexts (NSTextViews)
void setStandardControlFont(NSControl *control)
{
	[control setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
}

void uiDoFreeOnDealloc(NSMutableArray *m)
{
	[m enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSValue *v = (NSValue *) obj;

		uiFree([v pointerValue]);
	}];
	[m release];
}
