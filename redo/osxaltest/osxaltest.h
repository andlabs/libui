// 31 july 2015
#import <Cocoa/Cocoa.h>
#import <stdint.h>

@protocol tControl
@required
- (void)tAddToView:(NSView *)v;
- (uintmax_t)tAddToAutoLayoutDictionary:(NSMutableDictionary *)views keyNumber:(uintmax_t)n;
@end

@interface tWindowDelegate : NSObject<NSWindowDelegate>
- (void)tSetControl:(id<tControl>)cc;
- (void)tRelayout;
@end

/*
@interface tBox : NSObject<tControl>
- (id)tInitVertical:(BOOL)vert;
- (void)tAddControl:(id<tControl>)c stretchy:(BOOL)s;
@end
*/

@interface tButton : NSObject<tControl>
- (id)tInitWithText:(NSString *)text;
@end

extern NSString *tAutoLayoutKey(uintmax_t);
