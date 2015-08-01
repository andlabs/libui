// 31 july 2015
#import <Cocoa/Cocoa.h>
#import <stdint.h>

@protocol tControl
@required
- (void)tAddToView:(NSView *)v;
- (uintmax_t)tAddToAutoLayoutDictionary:(NSMutableDictionary *)views keyNumber:(uintmax_t)n;
@end

@interface tWindowDelegate : NSObject<NSWindowDelegate>
- (void)tSetControl:(NSObject<tControl> *)cc;
- (void)tRelayout;
@end

extern NSString *tAutoLayoutKey(uintmax_t);
