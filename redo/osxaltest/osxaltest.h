// 31 july 2015
#import <Cocoa/Cocoa.h>
#import <stdint.h>

typedef struct tAutoLayoutParams tAutoLayoutParams;

// TODO stretchy across both dimensions
// for a vertical box, the horizontal width should be variable
struct tAutoLayoutParams {
	NSView *view;
	BOOL attachLeft;
	BOOL attachTop;
	BOOL attachRight;
	BOOL attachBottom;
	NSString *nonStretchyWidthPredicate;
	NSString *nonStretchyHeightPredicate;
};

@protocol tControl
@required
- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v relayout:(BOOL)relayout;
- (void)tFillAutoLayout:(tAutoLayoutParams *)p;
- (void)tRelayout;
@end

@interface tWindow : NSObject<tControl>
- (void)tSetControl:(id<tControl>)cc;
- (void)tSetMargined:(BOOL)m;
- (void)tShow;
@end

@interface tBox : NSObject<tControl>
- (id)tInitVertical:(BOOL)vert spaced:(BOOL)sp;
- (void)tAddControl:(id<tControl>)c stretchy:(BOOL)s;
@end

@interface tButton : NSObject<tControl>
- (id)tInitWithText:(NSString *)text;
@end

@interface tEntry : NSObject<tControl>
@end

@interface tLabel : NSObject<tControl>
@end

@interface tSpinbox : NSObject<tControl>
@end

extern NSString *tAutoLayoutKey(uintmax_t);
