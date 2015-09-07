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

// TODOs
// - windows: SetPolyFillMode
// - os x: FillPath/EOFillPath functions
enum uiDrawFillMode {
	uiDrawFillModeWinding,
	// TODO rename to EvenOdd?
	uiDrawFillModeAlternate,
};

struct uiDrawStrokeParams {
	uiDrawLineCap Cap;
	uiDrawLineJoin Join;
	intmax_t Thickness;
	// TODO float for GDI?
	double MiterLimit;
};

void uiDrawBeginPathRGB(uiDrawContext *, uint8_t, uint8_t, uint8_t);
void uiDrawBeginPathRGBA(uiDrawContext *, uint8_t, uint8_t, uint8_t, uint8_t);

void uiDrawMoveTo(uiDrawContext *, intmax_t, intmax_t);
void uiDrawLineTo(uiDrawContext *, intmax_t, intmax_t);
void uiDrawRectangle(uiDrawContext *, intmax_t, intmax_t, intmax_t, intmax_t);
// notes: angles are both relative to 0 and go counterclockwise
void uiDrawArc(uiDrawContext *, intmax_t, intmax_t, intmax_t, double, double, int);
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

// arcs
// cairo_arc/arc_negative
// - parameters: center, radius, angle1 radians, angle2 radins
// - if angle2 < angle1, TODO
// - if angle2 > angle1, TODO
// - line segment from current point to beginning of arc
// - arc: clockwise, arc_negative: counterclockwise
// - circular
// GDI Arc/Pie
// - parameters: bounding box, start point, end point
// - current position not used/changed
// - either clockwise or counterclockwise
// - elliptical
// GDI ArcTo
// - same as Arc except line segment from current point to beginning of arc
// - there does not appear to be a PieTo
// GDI AngleArc
// - parameters: center, radius, angle1 degrees, angle2 degrees
// - line segment from current position to beginning of arc
// - counterclockwise
// - circular
// - can be used to draw pies too; MSDN example demonstrates
