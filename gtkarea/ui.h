// 4 september 2015

typedef struct uiArea uiArea;
typedef struct uiAreaHandler uiAreaHandler;
typedef struct uiAreaDrawParams uiAreaDrawParams;

typedef struct uiDrawContext uiDrawContext;

struct uiAreaHandler {
	void (*Draw)(uiAreaHandler *, uiArea *, uiAreaDrawParams *);
	uintmax_t (*HScrollMax)(uiAreaHandler *, uiArea *);
	uintmax_t (*VScrollMax)(uiAreaHandler *, uiArea *);
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
typedef struct uiDrawFillParams uiDrawFillParams;
typedef enum uiDrawLineCap uiDrawLineCap;
typedef enum uiDrawLineJoin uiDrawLineJoin;
typedef enum uiDrawFillMode uiDrawFillMode;

typedef uint32_t uiRGB;

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

// TODOs
// - windows: SetPolyFillMode
// - os x: FillPath/EOFillPath functions
enum uiDrawFillMode {
	uiDrawFillModeWinding,
	uiDrawFillModeAlternate,
};

struct uiDrawStrokeParams {
	uiRGB RGB;
	uiDrawLineCap Cap;
	uiDrawLineJoin Join;
	intmax_t Thickness;
	// TODO float for GDI?
	double MiterLimit;
};

struct uiDrawFillParams {
	uiRGB RGB;
	uiDrawFillMode FillMode;
};

void uiDrawMoveTo(uiDrawContext *, intmax_t, intmax_t);
void uiDrawLineTo(uiDrawContext *, intmax_t, intmax_t);
void uiDrawCloseFigure(uiDrawContext *);
void uiDrawStroke(uiDrawContext *, uiDrawStrokeParams *);

// path functions
// cairo			gdi						core graphics
// move_to		MoveToEx				MoveToPoint
// line_to			LineTo					AddLineToPoint
// arc			Arc/ArcTo/AngleArc/Ellipse	AddArc/AddArcToPoint/AddEllipseInRect
// arc_negative	Pie						AddArc/AddArcToPoint
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
