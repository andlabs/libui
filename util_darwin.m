// 7 april 2015
#import "uipriv_darwin.h"

// also fine for NSCells and NSTexts (NSTextViews)
void setStandardControlFont(NSControl *control)
{
	[control setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
}
