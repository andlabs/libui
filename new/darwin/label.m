// 9 april 2015
#import "uipriv_darwin.h"

struct label {
	uiLabel l;
	NSTextField *label;
};

static void destroy(void *data)
{
	struct label *l = (struct label *) data;

	uiFree(l);
}

static char *labelText(uiLabel *ll)
{
	struct label *l = (struct label *) ll;

	return uiDarwinNSStringToText([l->label stringValue]);
}

static void labelSetText(uiLabel *ll, const char *text)
{
	struct label *l = (struct label *) ll;

	[l->label setStringValue:toNSString(text)];
}

uiLabel *uiNewLabel(const char *text)
{
	struct label *l;

	l = uiNew(struct label);

	uiDarwinNewControl(uiControl(l), [NSTextField class], NO, NO, destroy, l);

	l->label = (NSTextField *) uiControlHandle(uiControl(l));

	[l->label setStringValue:toNSString(text)];
	[l->label setEditable:NO];
	[l->label setSelectable:NO];
	[l->label setDrawsBackground:NO];
	finishNewTextField(l->label, NO);

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
