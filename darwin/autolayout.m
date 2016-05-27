// 15 august 2015
#import "uipriv_darwin.h"

NSLayoutConstraint *mkConstraint(id view1, NSLayoutAttribute attr1, NSLayoutRelation relation, id view2, NSLayoutAttribute attr2, CGFloat multiplier, CGFloat c, NSString *desc)
{
	NSLayoutConstraint *constraint;

	constraint = [NSLayoutConstraint constraintWithItem:view1
		attribute:attr1
		relatedBy:relation
		toItem:view2
		attribute:attr2
		multiplier:multiplier
		constant:c];
	// apparently only added in 10.9
	if ([constraint respondsToSelector:@selector(setIdentifier:)])
		[((id) constraint) setIdentifier:desc];
	return constraint;
}

CGFloat uiDarwinMarginAmount(void *reserved)
{
	return 20.0;
}

CGFloat uiDarwinPaddingAmount(void *reserved)
{
	return 8.0;
}

// this is needed for NSSplitView to work properly; see http://stackoverflow.com/questions/34574478/how-can-i-set-the-position-of-a-nssplitview-nowadays-setpositionofdivideratind (stal in irc.freenode.net/#macdev came up with the exact combination)
// turns out it also works on NSTabView and NSBox too, possibly others!
// and for bonus points, it even seems to fix unsatisfiable-constraint-autoresizing-mask issues with NSTabView and NSBox too!!! this is nuts
void jiggleViewLayout(NSView *view)
{
	[view setNeedsLayout:YES];
	[view layoutSubtreeIfNeeded];
}

static CGFloat margins(int margined)
{
	if (!margined)
		return 0.0;
	return uiDarwinMarginAmount(NULL);
}

void singleChildConstraintsEstablish(struct singleChildConstraints *c, NSView *contentView, NSView *childView, BOOL hugsTrailing, BOOL hugsBottom, int margined, NSString *desc)
{
	CGFloat margin;

	margin = margins(margined);

	c->leadingConstraint = mkConstraint(contentView, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		childView, NSLayoutAttributeLeading,
		1, -margin,
		[desc stringByAppendingString:@" leading constraint"]);
	[contentView addConstraint:c->leadingConstraint];
	[c->leadingConstraint retain];

	c->topConstraint = mkConstraint(contentView, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		childView, NSLayoutAttributeTop,
		1, -margin,
		[desc stringByAppendingString:@" top constraint"]);
	[contentView addConstraint:c->topConstraint];
	[c->topConstraint retain];

	c->trailingConstraintGreater = mkConstraint(contentView, NSLayoutAttributeTrailing,
		NSLayoutRelationGreaterThanOrEqual,
		childView, NSLayoutAttributeTrailing,
		1, margin,
		[desc stringByAppendingString:@" trailing >= constraint"]);
	if (hugsTrailing)
		[c->trailingConstraintGreater setPriority:NSLayoutPriorityDefaultLow];
	[contentView addConstraint:c->trailingConstraintGreater];
	[c->trailingConstraintGreater retain];

	c->trailingConstraintEqual = mkConstraint(contentView, NSLayoutAttributeTrailing,
		NSLayoutRelationEqual,
		childView, NSLayoutAttributeTrailing,
		1, margin,
		[desc stringByAppendingString:@" trailing == constraint"]);
	if (!hugsTrailing)
		[c->trailingConstraintEqual setPriority:NSLayoutPriorityDefaultLow];
	[contentView addConstraint:c->trailingConstraintEqual];
	[c->trailingConstraintEqual retain];

	c->bottomConstraintGreater = mkConstraint(contentView, NSLayoutAttributeBottom,
		NSLayoutRelationGreaterThanOrEqual,
		childView, NSLayoutAttributeBottom,
		1, margin,
		[desc stringByAppendingString:@" bottom >= constraint"]);
	if (hugsBottom)
		[c->bottomConstraintGreater setPriority:NSLayoutPriorityDefaultLow];
	[contentView addConstraint:c->bottomConstraintGreater];
	[c->bottomConstraintGreater retain];

	c->bottomConstraintEqual = mkConstraint(contentView, NSLayoutAttributeBottom,
		NSLayoutRelationEqual,
		childView, NSLayoutAttributeBottom,
		1, margin,
		[desc stringByAppendingString:@" bottom == constraint"]);
	if (!hugsBottom)
		[c->bottomConstraintEqual setPriority:NSLayoutPriorityDefaultLow];
	[contentView addConstraint:c->bottomConstraintEqual];
	[c->bottomConstraintEqual retain];
}

void singleChildConstraintsRemove(struct singleChildConstraints *c, NSView *cv)
{
	if (c->leadingConstraint != nil) {
		[cv removeConstraint:c->leadingConstraint];
		[c->leadingConstraint release];
		c->leadingConstraint = nil;
	}
	if (c->topConstraint != nil) {
		[cv removeConstraint:c->topConstraint];
		[c->topConstraint release];
		c->topConstraint = nil;
	}
	if (c->trailingConstraintGreater != nil) {
		[cv removeConstraint:c->trailingConstraintGreater];
		[c->trailingConstraintGreater release];
		c->trailingConstraintGreater = nil;
	}
	if (c->trailingConstraintEqual != nil) {
		[cv removeConstraint:c->trailingConstraintEqual];
		[c->trailingConstraintEqual release];
		c->trailingConstraintEqual = nil;
	}
	if (c->bottomConstraintGreater != nil) {
		[cv removeConstraint:c->bottomConstraintGreater];
		[c->bottomConstraintGreater release];
		c->bottomConstraintGreater = nil;
	}
	if (c->bottomConstraintEqual != nil) {
		[cv removeConstraint:c->bottomConstraintEqual];
		[c->bottomConstraintEqual release];
		c->bottomConstraintEqual = nil;
	}
}

void singleChildConstraintsSetMargined(struct singleChildConstraints *c, int margined)
{
	CGFloat margin;

	margin = margins(margined);
	if (c->leadingConstraint != nil)
		[c->leadingConstraint setConstant:-margin];
	if (c->topConstraint != nil)
		[c->topConstraint setConstant:-margin];
	if (c->trailingConstraintGreater != nil)
		[c->trailingConstraintGreater setConstant:margin];
	if (c->trailingConstraintEqual != nil)
		[c->trailingConstraintEqual setConstant:margin];
	if (c->bottomConstraintGreater != nil)
		[c->bottomConstraintGreater setConstant:margin];
	if (c->bottomConstraintEqual != nil)
		[c->bottomConstraintEqual setConstant:margin];
}

// based on http://blog.bjhomer.com/2014/08/nsscrollview-and-autolayout.html because (as pointed out there) Apple's official guide is really only for iOS
void scrollViewConstraintsEstablish(struct scrollViewConstraints *c, NSScrollView *sv, BOOL hscroll, BOOL vscroll, NSString *desc)
{
	NSView *cv, *dv;
	NSLayoutRelation rel;

	scrollViewConstraintsRemove(c, sv);
	cv = [sv contentView];
	dv = [sv documentView];

	c->documentLeading = mkConstraint(dv, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		cv, NSLayoutAttributeLeading,
		1, 0,
		[desc stringByAppendingString:@"document leading constraint"]);
	[sv addConstraint:c->documentLeading];
	[c->documentLeading retain];

	c->documentTop = mkConstraint(dv, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		cv, NSLayoutAttributeTop,
		1, 0,
		[desc stringByAppendingString:@"document top constraint"]);
	[sv addConstraint:c->documentTop];
	[c->documentTop retain];

	c->hscroll = hscroll;
	rel = NSLayoutRelationGreaterThanOrEqual;
	if (!c->hscroll)
		rel = NSLayoutRelationEqual;
	c->documentTrailing = mkConstraint(dv, NSLayoutAttributeTrailing,
		rel,
		cv, NSLayoutAttributeTrailing,
		1, 0,
		[desc stringByAppendingString:@"document trailing constraint"]);
	[sv addConstraint:c->documentTrailing];
	[c->documentTrailing retain];

	c->vscroll = vscroll;
	rel = NSLayoutRelationGreaterThanOrEqual;
	if (!c->vscroll)
		rel = NSLayoutRelationEqual;
	c->documentBottom = mkConstraint(dv, NSLayoutAttributeBottom,
		rel,
		sv, NSLayoutAttributeBottom,
		1, 0,
		[desc stringByAppendingString:@"document bottom constraint"]);
	[sv addConstraint:c->documentBottom];
	[c->documentBottom retain];
}

void scrollViewConstraintsRemove(struct scrollViewConstraints *c, NSScrollView *sv)
{
	if (c->documentLeading != nil) {
		[sv removeConstraint:c->documentLeading];
		[c->documentLeading release];
		c->documentLeading = nil;
	}
	if (c->documentTop != nil) {
		[sv removeConstraint:c->documentTop];
		[c->documentTop release];
		c->documentTop = nil;
	}
	if (c->documentTrailing != nil) {
		[sv removeConstraint:c->documentTrailing];
		[c->documentTrailing release];
		c->documentTrailing = nil;
	}
	if (c->documentBottom != nil) {
		[sv removeConstraint:c->documentBottom];
		[c->documentBottom release];
		c->documentBottom = nil;
	}
}
