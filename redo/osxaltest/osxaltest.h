// 31 july 2015
#import <Cocoa/Cocoa.h>
#import <stdint.h>

@protocol tControl
@required
- (void)tAddToView:(NSView *)v;
//TODO- (uintmax_t)tAddToAutoLayoutDictionary:(NSMutableDictionary *)views keyNumber:(uintmax_t)n;
- (void)tFillAutoLayoutHorz:(NSMutableString *)horz
	vert:(NSMutableString *)vert
	extra:(NSMutableArray *)extra
	extraVert:(NSMutableArray *)extraVert
	views:(NSMutableDictionary *)views;
@end

@interface tWindow : NSObject<tControl>
- (void)tSetControl:(id<tControl>)cc;
- (void)tSetMargined:(BOOL)m;
- (void)tShow;
// TODO this should probably in tControl
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

@interface tSpinbox : NSObject<tControl>
@end

extern NSString *tAutoLayoutKey(uintmax_t);
