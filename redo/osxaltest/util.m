// 1 august 2015
#import "osxaltest.h"

NSString *tAutoLayoutKey(uintmax_t n)
{
	return [NSString stringWithFormat:@"[view%ju]", n];
}
