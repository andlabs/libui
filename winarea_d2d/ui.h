// 4 september 2015

typedef struct uiArea uiArea;
typedef struct uiAreaHandler uiAreaHandler;
typedef struct uiAreaDrawParams uiAreaDrawParams;
typedef struct uiAreaMouseEvent uiAreaMouseEvent;
typedef struct uiAreaKeyEvent uiAreaKeyEvent;

typedef struct uiDrawContext uiDrawContext;

struct uiAreaHandler {
	void (*Draw)(uiAreaHandler *, uiArea *, uiAreaDrawParams *);
	uintmax_t (*HScrollMax)(uiAreaHandler *, uiArea *);
	uintmax_t (*VScrollMax)(uiAreaHandler *, uiArea *);
	int (*RedrawOnResize)(uiAreaHandler *, uiArea *);
	void (*MouseEvent)(uiAreaHandler *, uiArea *, uiAreaMouseEvent *);
	void (*DragBroken)(uiAreaHandler *, uiArea *);
	int (*KeyEvent)(uiAreaHandler *, uiArea *, uiAreaKeyEvent *);
};

struct uiAreaDrawParams {
	uiDrawContext *Context;

	intmax_t ClientWidth;
	intmax_t ClientHeight;

	intmax_t ClipX;
	intmax_t ClipY;
	intmax_t ClipWidth;
	intmax_t ClipHeight;

	int DPIX;
	int DPIY;

	intmax_t HScrollPos;
	intmax_t VScrollPos;
};

// TODO proper sources
// TODO dotting/dashing

typedef struct uiDrawPath uiDrawPath;
typedef struct uiDrawStrokeParams uiDrawStrokeParams;
typedef enum uiDrawLineCap uiDrawLineCap;
typedef enum uiDrawLineJoin uiDrawLineJoin;
typedef enum uiDrawFillMode uiDrawFillMode;

enum uiDrawLineCap {
	uiDrawLineCapFlat,
	uiDrawLineCapRound,
	uiDrawLineCapSquare,
};

enum uiDrawLineJoin {
	uiDrawLineJoinMiter,
	uiDrawLineJoinRound,
	uiDrawLineJoinBevel,
};

// this is the default for botoh cairo and GDI
// Core Graphics doesn't explicitly specify a default, but NSBezierPath allows you to choose one, and this is the initial value
// so we're good to use it too!
#define uiDrawDefaultMiterLimit 10.0

enum uiDrawFillMode {
	uiDrawFillModeWinding,
	uiDrawFillModeAlternate,
};

struct uiDrawStrokeParams {
	uiDrawLineCap Cap;
	uiDrawLineJoin Join;
	intmax_t Thickness;
	double MiterLimit;
};

uiDrawPath *uiDrawNewPath(uiDrawFillMode);
void uiDrawFreePath(uiDrawPath *);

void uiDrawPathNewFigure(uiDrawPath *, double, double);
void uiDrawPathNewFigureWithArc(uiDrawPath *, double, double, double, double, double);
void uiDrawPathLineTo(uiDrawPath *, double, double);
// notes: angles are both relative to 0 and go counterclockwise
// TODO is the initial line segment on cairo and OS X a proper join?
void uiDrawPathArcTo(uiDrawPath *, double, double, double, double, double);
void uiDrawPathBezierTo(uiDrawPath *, double, double, double, double, double, double);
// TODO quadratic bezier
void uiDrawPathCloseFigure(uiDrawPath *);

void uiDrawPathAddRectangle(uiDrawPath *, double, double, double, double);

void uiDrawPathEnd(uiDrawPath *);

void uiDrawStroke(uiDrawContext *, uiDrawPath *, uiDrawStrokeParams *);
void uiDrawFill(uiDrawContext *, uiDrawPath *);

// TODO primitives:
// - rounded rectangles
// - elliptical arcs
// - quadratic bezier curves

typedef enum uiModifiers uiModifiers;

enum uiModifiers {
	uiModifierCtrl = 1 << 0,
	uiModifierAlt = 1 << 1,
	uiModifierShift = 1 << 2,
	uiModifierSuper = 1 << 3,
};

struct uiAreaMouseEvent {
	intmax_t X;
	intmax_t Y;

	intmax_t ClientWidth;
	intmax_t ClientHeight;
	intmax_t HScrollPos;
	intmax_t VScrollPos;

	uintmax_t Down;
	uintmax_t Up;

	uintmax_t Count;

	uiModifiers Modifiers;

	uint64_t Held1To64;
};

typedef enum uiExtKey uiExtKey;

enum uiExtKey {
	uiExtKeyEscape = 1,
	uiExtKeyInsert,			// equivalent to "Help" on Apple keyboards
	uiExtKeyDelete,
	uiExtKeyHome,
	uiExtKeyEnd,
	uiExtKeyPageUp,
	uiExtKeyPageDown,
	uiExtKeyUp,
	uiExtKeyDown,
	uiExtKeyLeft,
	uiExtKeyRight,
	uiExtKeyF1,			// F1..F12 are guaranteed to be consecutive
	uiExtKeyF2,
	uiExtKeyF3,
	uiExtKeyF4,
	uiExtKeyF5,
	uiExtKeyF6,
	uiExtKeyF7,
	uiExtKeyF8,
	uiExtKeyF9,
	uiExtKeyF10,
	uiExtKeyF11,
	uiExtKeyF12,
	uiExtKeyN0,			// numpad keys; independent of Num Lock state
	uiExtKeyN1,			// N0..N9 are guaranteed to be consecutive
	uiExtKeyN2,
	uiExtKeyN3,
	uiExtKeyN4,
	uiExtKeyN5,
	uiExtKeyN6,
	uiExtKeyN7,
	uiExtKeyN8,
	uiExtKeyN9,
	uiExtKeyNDot,
	uiExtKeyNEnter,
	uiExtKeyNAdd,
	uiExtKeyNSubtract,
	uiExtKeyNMultiply,
	uiExtKeyNDivide,
};

struct uiAreaKeyEvent {
	char Key;
	uiExtKey ExtKey;
	uiModifiers Modifier;

	uiModifiers Modifiers;

	int Up;
};
