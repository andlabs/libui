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

void setHuggingPri(NSView *view, NSLayoutPriority priority, NSLayoutConstraintOrientation orientation)
{
	[view setContentHuggingPriority:priority forOrientation:orientation];
}

NSLayoutPriority horzHuggingPri(NSView *view)
{
	return [view contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationHorizontal];
}

void setHorzHuggingPri(NSView *view, NSLayoutPriority priority)
{
	[view setContentHuggingPriority:priority forOrientation:NSLayoutConstraintOrientationHorizontal];
}

NSLayoutPriority vertHuggingPri(NSView *view)
{
	return [view contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationVertical];
}

void setVertHuggingPri(NSView *view, NSLayoutPriority priority)
{
	[view setContentHuggingPriority:priority forOrientation:NSLayoutConstraintOrientationVertical];
}

// precondition: subview is a subview of superview already
void layoutSingleView(NSView *superview, NSView *subview, int margined, NSString *desc)
{
	NSLayoutConstraint *constraint;
	CGFloat margin;

	[superview removeConstraints:[superview constraints]];

	// don't hug if needed (this fixes things like the Disabled Tab on Page 2 of the test program not being the one that resizes with the window
	setHorzHuggingPri(subview, NSLayoutPriorityDefaultLow);
	setVertHuggingPri(subview, NSLayoutPriorityDefaultLow);

	margin = 0;
	if (margined)
		margin = 20;		// TODO named constant

	constraint = mkConstraint(subview, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		superview, NSLayoutAttributeLeading,
		1, margin,
		[desc stringByAppendingString:@" single child horizontal leading"]);
	[superview addConstraint:constraint];

	constraint = mkConstraint(superview, NSLayoutAttributeTrailing,
		NSLayoutRelationEqual,
		subview, NSLayoutAttributeTrailing,
		1, margin,
		[desc stringByAppendingString:@" single child horizontal trailing"]);
	[superview addConstraint:constraint];

	constraint = mkConstraint(subview, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		superview, NSLayoutAttributeTop,
		1, margin,
		[desc stringByAppendingString:@" single child top"]);
	[superview addConstraint:constraint];

	constraint = mkConstraint(superview, NSLayoutAttributeBottom,
		NSLayoutRelationEqual,
		subview, NSLayoutAttributeBottom,
		1, margin,
		[desc stringByAppendingString:@" single child bottom"]);
	[superview addConstraint:constraint];
}

// via https://developer.apple.com/library/mac/documentation/UserExperience/Conceptual/AutolayoutPG/WorkingwithScrollViews.html#//apple_ref/doc/uid/TP40010853-CH24-SW1
NSMutableArray *layoutScrollViewContents(NSScrollView *sv, BOOL noHScroll, BOOL noVScroll, NSString *desc)
{
	NSView *dv;
	NSLayoutConstraint *constraint;
	NSMutableArray *array;

	dv = [sv documentView];

	array = [NSMutableArray new];

	constraint = mkConstraint(dv, NSLayoutAttributeLeading,
		NSLayoutRelationEqual,
		sv, NSLayoutAttributeLeading,
		1, 0,
		[desc stringByAppendingString:@" scroll view horizontal leading"]);
	[array addObject:constraint];
	[sv addConstraint:constraint];

	constraint = mkConstraint(dv, NSLayoutAttributeTrailing,
		NSLayoutRelationEqual,
		sv, NSLayoutAttributeTrailing,
		1, 0,
		[desc stringByAppendingString:@" scroll view horizontal trailing"]);
	[array addObject:constraint];
	[sv addConstraint:constraint];

	constraint = mkConstraint(dv, NSLayoutAttributeTop,
		NSLayoutRelationEqual,
		sv, NSLayoutAttributeTop,
		1, 0,
		[desc stringByAppendingString:@" scroll view top"]);
	[array addObject:constraint];
	[sv addConstraint:constraint];

	constraint = mkConstraint(dv, NSLayoutAttributeBottom,
		NSLayoutRelationEqual,
		sv, NSLayoutAttributeBottom,
		1, 0,
		[desc stringByAppendingString:@" scroll view bottom"]);
	[array addObject:constraint];
	[sv addConstraint:constraint];

	if (noHScroll) {
		constraint = mkConstraint(dv, NSLayoutAttributeWidth,
			NSLayoutRelationEqual,
			sv, NSLayoutAttributeWidth,
			1, 0,
			[desc stringByAppendingString:@" scroll view width"]);
		[array addObject:constraint];
		[sv addConstraint:constraint];
	}

	if (noVScroll) {
		constraint = mkConstraint(dv, NSLayoutAttributeHeight,
			NSLayoutRelationEqual,
			sv, NSLayoutAttributeHeight,
			1, 0,
			[desc stringByAppendingString:@" scroll view height"]);
		[array addObject:constraint];
		[sv addConstraint:constraint];
	}

	return array;
}
