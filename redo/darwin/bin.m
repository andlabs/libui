// 28 april 2015
#import "uipriv_darwin.h"

// This is a uiControl wrapper a la GtkBin on GTK+.
// It serves the function of tabPage on Windows: it allows uiWindow and uiTab to give their children a real uiControl as a parent while not screwing with the internal NSView structure of those uiControls.
// It also provides margins.

struct bin {
	uiControl c;
	NSView *view;
	uiControl *child;
	int margined;
	NSArray *hconstraint;
	NSArray *vconstraint;
};

uiDefineControlType(bin, binType, struct bin)

static uintptr_t binHandle(uiControl *c)
{
	struct bin *b = (struct bin *) c;

	return (uintptr_t) (b->view);
}

uiControl *newBin(void)
{
	struct bin *b;

	b = (struct bin *) uiNewControl(binType());

	// a simple NSView will do fine
	b->view = [[NSView alloc] initWithFrame:NSZeroRect];
	uiDarwinMakeSingleViewControl(uiControl(b), b->view, NO);

	uiControl(b)->Handle = binHandle;

	return uiControl(b);
}

void binSetChild(uiControl *c, uiControl *child)
{
	struct bin *b = (struct bin *) c;
	NSView *binView, *childView;
	NSDictionary *views;

	binView = (NSView *) uiControlHandle(uiControl(b));
	if (b->child != NULL) {
		[binView removeConstraints:b->hconstraint];
		[binView removeConstraints:b->vconstraint];
		[b->hconstraint release];
		[b->vconstraint release];
		childView = (NSView *) uiControlHandle(b->child);
		[childView removeFromSuperview];
	}
	b->child = child;
	if (b->child != NULL) {
		uiControlSetParent(b->child, uiControl(b));
		childView = (NSView *) uiControlHandle(b->child);
		[childView setTranslatesAutoresizingMaskIntoConstraints:NO];
		views = NSDictionaryOfVariableBindings(childView);
		b->hconstraint = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[childView]|" options:0 metrics:nil views:views];
		b->vconstraint = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[childView]|" options:0 metrics:nil views:views];
		[binView addConstraints:b->hconstraint];
		[binView addConstraints:b->vconstraint];
	}
}

int binMargined(uiControl *c)
{
	struct bin *b = (struct bin *) c;

	return b->margined;
}

void binSetMargined(uiControl *c, int margined)
{
	struct bin *b = (struct bin *) c;

	b->margined = margined;
	// TODO use auto layout
}
