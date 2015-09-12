// 4 september 2015

typedef struct uiArea uiArea;
typedef struct uiAreaHandler uiAreaHandler;
typedef struct uiAreaDrawParams uiAreaDrawParams;
typedef struct uiAreaMouseEvent uiAreaMouseEvent;

typedef struct uiDrawContext uiDrawContext;

struct uiAreaHandler {
	void (*Draw)(uiAreaHandler *, uiArea *, uiAreaDrawParams *);
	uintmax_t (*HScrollMax)(uiAreaHandler *, uiArea *);
	uintmax_t (*VScrollMax)(uiAreaHandler *, uiArea *);
	int (*RedrawOnResize)(uiAreaHandler *, uiArea *);
	void (*MouseEvent)(uiAreaHandler *, uiArea *, uiAreaMouseEvent *);
	void (*DragBroken)(uiAreaHandler *, uiArea *);
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
	// TODO rename to EvenOdd?
	uiDrawFillModeAlternate,
};

struct uiDrawStrokeParams {
	uiDrawLineCap Cap;
	uiDrawLineJoin Join;
	intmax_t Thickness;
	double MiterLimit;
};

void uiDrawBeginPathRGB(uiDrawContext *, uint8_t, uint8_t, uint8_t);
// TODO verify these aren't alpha premultiplied anywhere
void uiDrawBeginPathRGBA(uiDrawContext *, uint8_t, uint8_t, uint8_t, uint8_t);

void uiDrawMoveTo(uiDrawContext *, intmax_t, intmax_t);
void uiDrawLineTo(uiDrawContext *, intmax_t, intmax_t);
void uiDrawRectangle(uiDrawContext *, intmax_t, intmax_t, intmax_t, intmax_t);
// notes: angles are both relative to 0 and go counterclockwise
void uiDrawArcTo(uiDrawContext *, intmax_t, intmax_t, intmax_t, double, double, int);
// TODO behavior when there is no initial point on Windows and OS X
void uiDrawBezierTo(uiDrawContext *, intmax_t, intmax_t, intmax_t, intmax_t, intmax_t, intmax_t);
void uiDrawCloseFigure(uiDrawContext *);

void uiDrawStroke(uiDrawContext *, uiDrawStrokeParams *);
void uiDrawFill(uiDrawContext *, uiDrawFillMode);

// path functions
// cairo			gdi						core graphics
// move_to		MoveToEx				MoveToPoint
// line_to			LineTo					AddLineToPoint
// arc			Arc/ArcTo/AngleArc/Pie		AddArc/AddArcToPoint/AddEllipseInRect
// arc_negative	Arc/ArcTo/AngleArc/Pie		AddArc/AddArcToPoint
// curve_to		PolyBezier/PolyBezierTo		AddCurveToPoint
// rectangle		Rectangle					AddRect
// [arc functions?]	Chord					[arc functions?]
// [combination]	RoundRect				[same way as cairo?]
// [TODO pango]	TextOut/ExtTextOut			[TODO core text]
// [TODO]		[TODO]					AddQuadCurveToPoint

// on sources:
// cairo:
// - RGB
// - RGBA
// - images
// - linear gradients, RGB or RGBA
// - rounded gradients, RGB or RGBA
// gdi:
// - RGB
// - hatches
// - images
// we can create a linear gradient image, but RGB only, and of finite size
// core graphics:
// - arbitrary patterns
// - solid colors, arbitrary spaces
// - shadows

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
