// 15 august 2015
#import "uipriv_darwin.h"

void addConstraint(NSView *view, NSString *constraint, NSDictionary *metrics, NSDictionary *views)
{
	NSArray *constraints;

	constraints = [NSLayoutConstraint constraintsWithVisualFormat:constraint
		options:0
		metrics:metrics
		views:views];
	[view addConstraints:constraints];
}

NSLayoutPriority horzHuggingPri(NSView *view)
{
	return [view contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationHorizontal];
}

NSLayoutPriority vertHuggingPri(NSView *view)
{
	return [view contentHuggingPriorityForOrientation:NSLayoutConstraintOrientationVertical];
}

void setHuggingPri(NSView *view, NSLayoutPriority priority, NSLayoutConstraintOrientation orientation)
{
	[view setContentHuggingPriority:priority forOrientation:orientation];
}

// precondition: constraints must have been removed from superview already
void layoutSingleView(NSView *superview, NSView *subview, int margined)
{
	NSDictionary *views;
	NSString *constraint;

	views = NSDictionaryOfVariableBindings(subview);

	constraint = @"H:|[subview]|";
	if (margined)
		constraint = @"H:|-[subview]-|";
	addConstraint(superview, constraint, nil, views);

	constraint = @"V:|[subview]|";
	if (margined)
		constraint = @"V:|-[subview]-|";
	addConstraint(superview, constraint, nil, views);
}

// use the fitting size, not the intrinsic content size, for the case of recursive views without an intrinsic content size
NSSize fittingAlignmentSize(NSView *view)
{
	NSSize s;
	NSRect r;

	s = [view fittingSize];
	// the fitting size is for a frame rect; we need an alignment rect
	r = NSMakeRect(0, 0, s.width, s.height);
	r = [view alignmentRectForFrame:r];
	return r.size;
}
