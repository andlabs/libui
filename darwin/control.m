// 16 august 2015
#import "uipriv_darwin.h"

void uiDarwinControlSetSuperview(uiControl *c, NSView *superview)
{
	(*(c->SetSuperview))(c, superview);
}

void uiDarwinSetControlFont(NSControl *c, NSControlSize size)
{
	[c setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:size]]];
}

#define uiDarwinControlSignature 0x44617277

uiDarwinControl *uiDarwinNewControl(size_t n, uint32_t typesig, const char *typenamestr)
{
	return uiDarwinControl(uiAllocControl(n, uiDarwinControlSignature, typesig, typenamestr));
}
