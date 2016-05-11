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
