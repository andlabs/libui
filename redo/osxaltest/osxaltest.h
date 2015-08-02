// 31 july 2015
#import <Cocoa/Cocoa.h>
#import <stdint.h>

typedef struct tAutoLayoutParams tAutoLayoutParams;

struct tAutoLayoutParams {
	NSMutableArray *horz;
	NSMutableArray *vert;
	NSMutableArray *extra;		// TODO make extraHorz and return BOOL NSNumber logic
	NSMutableArray *extraVert;
	NSMutableDictionary *views;
	uintmax_t n;
	BOOL stretchy;
	BOOL stretchyVert;
	BOOL firstStretchy;
	uintmax_t stretchyTo;
};

@protocol tControl
@required
- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v;
- (void)tFillAutoLayout:(tAutoLayoutParams *)p;
- (void)tRelayout;
@end

@interface tWindow : NSObject<tControl>
- (void)tSetControl:(id<tControl>)cc;
- (void)tSetMargined:(BOOL)m;
- (void)tShow;
@end

@interface tBox : NSObject<tControl>
- (id)tInitVertical:(BOOL)vert;
- (void)tAddControl:(id<tControl>)c stretchy:(BOOL)s;
@end

@interface tButton : NSObject<tControl>
- (id)tInitWithText:(NSString *)text;
@end

@interface tSpinbox : NSObject<tControl>
@end

extern NSString *tAutoLayoutKey(uintmax_t);
