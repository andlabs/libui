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

void layoutSingleView(NSView *superview, NSView *subview, int margined)
{
	NSDictionary *views;
	NSString *constraint;

	[superview removeConstraints:[superview constraints]];

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
