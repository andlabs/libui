// 7 june 2016
#import "uipriv_darwin.h"

@interface formChild : NSObject
@property uiControl *c;
@property (strong) NSTextField *label;
@property BOOL stretchy;
@property NSLayoutPriority oldHorzHuggingPri;
@property NSLayoutPriority oldVertHuggingPri;
@property (strong) NSLayoutConstraint *baseline;
- (NSView *)view;
@end

@interface formView : NSView {
	uiForm *f;
	NSMutableArray *children;
	int padded;
	uintmax_t nStretchy;

	NSLayoutConstraint *first;
	NSMutableArray *inBetweens;
	NSLayoutConstraint *last;
}
- (id)initWithF:(uiForm *)ff;
- (void)onDestroy;
- (void)removeOurConstraints;
- (void)syncEnableStates:(int)enabled;
- (CGFloat)paddingAmount;
- (void)establishOurConstraints;
- (void)append:(NSString *)label c:(uiControl *)c stretchy:(int)stretchy;
//TODO- (void)delete:(uintmax_t)n;
- (int)isPadded;
- (void)setPadded:(int)p;
- (BOOL)hugsTrailing;
- (BOOL)hugsBottom;
@end

struct uiForm {
	uiDarwinControl c;
	formView *view;
};

@implementation formChild

- (NSView *)view
{
	return (NSView *) uiControlHandle(self.c);
}

@end

@implementation formView

- (id)initWithF:(uiForm *)ff
{
	self = [super initWithFrame:NSZeroRect];
	if (self != nil) {
		self->f = ff;
		self->padded = 0;
		self->children = [NSMutableArray new];
		self->nStretchy = 0;

		self->inBetweens = [NSMutableArray new];
	}
	return self;
}

- (void)onDestroy
{
	formChild *fc;

	[self removeOurConstraints];
	[self->inBetweens release];

	for (fc in self->children) {
		[self removeConstraint:fc.baseline];
		fc.baseline = nil;
		uiControlSetParent(fc.c, NULL);
		uiDarwinControlSetSuperview(uiDarwinControl(fc.c), nil);
		uiControlDestroy(fc.c);
		[fc.label removeFromSuperview];
		fc.label = nil;
	}
	[self->children release];
}

- (void)removeOurConstraints
{
	if (self->first != nil) {
		[self removeConstraint:self->first];
		[self->first release];
		self->first = nil;
	}
	if ([self->inBetweens count] != 0) {
		[self removeConstraints:self->inBetweens];
		[self->inBetweens removeAllObjects];
	}
	if (self->last != nil) {
		[self removeConstraint:self->last];
		[self->last release];
		self->last = nil;
	}
}

- (void)syncEnableStates:(int)enabled
{
	formChild *fc;

	for (fc in self->children)
		uiDarwinControlSyncEnableState(uiDarwinControl(fc.c), enabled);
}

- (CGFloat)paddingAmount
{
	if (!self->padded)
		return 0.0;
	return uiDarwinPaddingAmount(NULL);
}

- (void)establishOurConstraints
{
	formChild *fc;
	CGFloat padding;
	NSView *prev, *prevlabel;;
	NSLayoutConstraint *c;
	NSLayoutRelation relation;

	[self removeOurConstraints];
	if ([self->children count] == 0)
		return;
	padding = [self paddingAmount];

	// first arrange the main controls vertically
	prev = nil;
	for (fc in self->children) {
		if (prev == nil) {		// first control; tie to top
			self->first = mkConstraint(self, NSLayoutAttributeTop,
				NSLayoutRelationEqual,
				[fc view], NSLayoutAttributeTop,
				1, 0,
				@"uiForm first child top constraint");
			[self addConstraint:self->first];
			[self->first retain];
			prev = [fc view];
			continue;
		}
		// not first; tie to previous
		c = mkConstraint(prev, NSLayoutAttributeBottom,
			NSLayoutRelationEqual,
			[fc view], NSLayoutAttributeTop,
			1, -padding,
			@"uiForm middle vertical constraint");
		[self addConstraint:c];
		[self->inBetweens addObject:c];
		prev = [fc view];
	}
	// and the last one
	self->last = mkConstraint(prev, NSLayoutAttributeBottom,
		NSLayoutRelationEqual,
		self, NSLayoutAttributeBottom,
		1, 0,
		@"uiForm last child bottom constraint");
	[self addConstraint:self->last];
	[self->last retain];

	// we don't arrange the labels vertically; that's done when we add the control since those constraints don't need to change (they just need to be at their baseline)
}

- (void)append:(NSString *)label c:(uiControl *)c stretchy:(int)stretchy
{
	formChild *fc;
	NSLayoutPriority priority;
	NSLayoutAttribute attribute;
	uintmax_t oldnStretchy;

	fc = [formChild new];
	fc.c = c;
	fc.label = newLabel(label);
	[fc.label setTranslatesAutoresizingMaskIntoConstraints:NO];
	// and make the label no larger than it needs to be
	[fc.label setContentHuggingPriority:NSLayoutPriorityRequired forOrientation:NSLayoutConstraintOrientationHorizontal];
	[fc.label setContentHuggingPriority:NSLayoutPriorityRequired forOrientation:NSLayoutConstraintOrientationVertical];
	[self addSubview:fc.label];
	fc.stretchy = stretchy;
	fc.oldHorzHuggingPri = uiDarwinControlHuggingPriority(uiDarwinControl(fc.c), NSLayoutConstraintOrientationHorizontal);
	fc.oldVertHuggingPri = uiDarwinControlHuggingPriority(uiDarwinControl(fc.c), NSLayoutConstraintOrientationVertical);

	uiControlSetParent(fc.c, uiControl(self->f));
	uiDarwinControlSetSuperview(uiDarwinControl(fc.c), self);
	uiDarwinControlSyncEnableState(uiDarwinControl(fc.c), uiControlEnabledToUser(uiControl(self->f)));

	// if a control is stretchy, it should not hug vertically
	// otherwise, it should *forcibly* hug
	if (fc.stretchy)
		priority = NSLayoutPriorityDefaultLow;
	else
		// LONGTERM will default high work?
		priority = NSLayoutPriorityRequired;
	uiDarwinControlSetHuggingPriority(uiDarwinControl(fc.c), priority, NSLayoutConstraintOrientationVertical);
	// make sure controls don't hug their horizontal direction so they fill the width of the view
	uiDarwinControlSetHuggingPriority(uiDarwinControl(fc.c), NSLayoutPriorityDefaultLow, NSLayoutConstraintOrientationHorizontal);

	// and constrain the baselines to position the label vertically
	// if the view is a scroll view, align tops, not baselines
	// this is what Interface Builder does
	attribute = NSLayoutAttributeBaseline;
	if ([[fc view] isKindOfClass:[NSScrollView class]])
		attribute = NSLayoutAttributeTop;
	fc.baseline = mkConstraint(fc.label, attribute,
		NSLayoutRelationEqual,
		[fc view], attribute,
		1, 0,
		@"uiForm baseline constraint");
	[self addConstraint:fc.baseline];

	[self->children addObject:fc];

	[self establishOurConstraints];
	if (fc.stretchy) {
		oldnStretchy = self->nStretchy;
		self->nStretchy++;
		if (oldnStretchy == 0)
			uiDarwinNotifyEdgeHuggingChanged(uiDarwinControl(self->f));
	}

	[fc release];		// we don't need the initial reference now
}

//TODO- (void)delete:(uintmax_t)n

- (int)isPadded
{
	return self->padded;
}

- (void)setPadded:(int)p
{
	CGFloat padding;
	NSLayoutConstraint *c;

	self->padded = p;
	padding = [self paddingAmount];
	for (c in self->inBetweens)
		[c setConstant:-padding];
}

- (BOOL)hugsTrailing
{
	return YES;			// always hug trailing
}

- (BOOL)hugsBottom
{
	// only hug if we have stretchy
	return self->nStretchy != 0;
}

@end

static void uiFormDestroy(uiControl *c)
{
	uiForm *f = uiForm(c);

	[f->view onDestroy];
	[f->view release];
	uiFreeControl(uiControl(f));
}

uiDarwinControlDefaultHandle(uiForm, view)
uiDarwinControlDefaultParent(uiForm, view)
uiDarwinControlDefaultSetParent(uiForm, view)
uiDarwinControlDefaultToplevel(uiForm, view)
uiDarwinControlDefaultVisible(uiForm, view)
uiDarwinControlDefaultShow(uiForm, view)
uiDarwinControlDefaultHide(uiForm, view)
uiDarwinControlDefaultEnabled(uiForm, view)
uiDarwinControlDefaultEnable(uiForm, view)
uiDarwinControlDefaultDisable(uiForm, view)

static void uiFormSyncEnableState(uiDarwinControl *c, int enabled)
{
	uiForm *f = uiForm(c);

	if (uiDarwinShouldStopSyncEnableState(uiDarwinControl(f), enabled))
		return;
	[f->view syncEnableStates:enabled];
}

uiDarwinControlDefaultSetSuperview(uiForm, view)

static BOOL uiFormHugsTrailingEdge(uiDarwinControl *c)
{
	uiForm *f = uiForm(c);

	return [f->view hugsTrailing];
}

static BOOL uiFormHugsBottom(uiDarwinControl *c)
{
	uiForm *f = uiForm(c);

	return [f->view hugsBottom];
}

static void uiFormChildEdgeHuggingChanged(uiDarwinControl *c)
{
	uiForm *f = uiForm(c);

	[f->view establishOurConstraints];
}

uiDarwinControlDefaultHuggingPriority(uiForm, view)
uiDarwinControlDefaultSetHuggingPriority(uiForm, view)

void uiFormAppend(uiForm *f, const char *label, uiControl *c, int stretchy)
{
	// LONGTERM on other platforms
	// or at leat allow this and implicitly turn it into a spacer
	if (c == NULL)
		userbug("You cannot add NULL to a uiForm.");
	[f->view append:toNSString(label) c:c stretchy:stretchy];
}

int uiFormPadded(uiForm *f)
{
	return [f->view isPadded];
}

void uiFormSetPadded(uiForm *f, int padded)
{
	[f->view setPadded:padded];
}

uiForm *uiNewForm(void)
{
	uiForm *f;

	uiDarwinNewControl(uiForm, f);

	f->view = [[formView alloc] initWithF:f];

	return f;
}
