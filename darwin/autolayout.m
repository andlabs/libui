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

static CGFloat margins(int margined)
{
	if (!margined)
		return 0.0;
	return 20.0;		// TODO named constant
}

void singleChildConstraintsEstablish(struct singleChildConstraints *c, NSView *contentView, NSView *childView, BOOL hugsTrailing, BOOL hugsBottom, int margined, NSString *desc)
{
	CGFloat margin;
	NSLayoutRelation relation;

	margin = margins(margined);

	c->leadingConstraint = mkConstraint(contentView, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		childView, NSLayoutAttributeLeading,
		1, -margin,
		[desc stringByAppendingString:@" leading constraint"]);
	[contentView addConstraint:c->leadingConstraint];

	c->topConstraint = mkConstraint(contentView, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		childView, NSLayoutAttributeTop,
		1, -margin,
		[desc stringByAppendingString:@" top constraint"]);
	[contentView addConstraint:c->topConstraint];

	relation = NSLayoutRelationGreaterThanOrEqual;
	if (hugsTrailing)
		relation = NSLayoutRelationEqual;
	c->trailingConstraint = mkConstraint(contentView, NSLayoutAttributeTrailing,
		relation,
		childView, NSLayoutAttributeTrailing,
		1, margin,
		[desc stringByAppendingString:@" trailing constraint"]);
	[contentView addConstraint:c->trailingConstraint];

	relation = NSLayoutRelationGreaterThanOrEqual;
	if (hugsBottom)
		relation = NSLayoutRelationEqual;
	c->bottomConstraint = mkConstraint(contentView, NSLayoutAttributeBottom,
		relation,
		childView, NSLayoutAttributeBottom,
		1, margin,
		[desc stringByAppendingString:@" bottom constraint"]);
	[contentView addConstraint:c->bottomConstraint];
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
	if (c->trailingConstraint != nil) {
		[cv removeConstraint:c->trailingConstraint];
		[c->trailingConstraint release];
		c->trailingConstraint = nil;
	}
	if (c->bottomConstraint != nil) {
		[cv removeConstraint:c->bottomConstraint];
		[c->bottomConstraint release];
		c->bottomConstraint = nil;
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
	if (c->trailingConstraint != nil)
		[c->trailingConstraint setConstant:margin];
	if (c->bottomConstraint != nil)
		[c->bottomConstraint setConstant:margin];
}
