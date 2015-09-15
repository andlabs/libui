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

void uiDrawBeginPathRGB(uiDrawContext *, uint8_t, uint8_t, uint8_t);
// TODO verify these aren't alpha premultiplied anywhere
void uiDrawBeginPathRGBA(uiDrawContext *, uint8_t, uint8_t, uint8_t, uint8_t);
// TODO uiDrawBeginTextRGB(/RGBA?)

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

// TODO primitives:
// - rounded rectangles
// - elliptical arcs
// - quadratic bezier curves

typedef struct uiDrawFont uiDrawFont;
typedef struct uiDrawFontSpec uiDrawFontSpec;
typedef struct uiDrawTextBlockParams uiDrawTextBlockParams;
typedef struct uiDrawFontMetrics uiDrawFontMetrics;
typedef enum uiDrawTextWeight uiDrawTextWeight;
typedef enum uiDrawTextAlign uiDrawTextAlign;

enum uiDrawTextWeight {
	uiDrawTextWeightThin,
	uiDrawTextWeightExtraLight,
	uiDrawTextWeightLight,
	uiDrawTextWeightNormal,
	uiDrawTextWeightMedium,
	uiDrawTextWeightSemiBold,
	uiDrawTextWeightBold,
	uiDrawTextWeightExtraBold,
	uiDrawTextWeightHeavy,
};

struct uiDrawFontSpec {
	const char *Family;
	uintmax_t PointSize;
	uiDrawTextWeight Weight;
	int Italic;			// always prefers true italics over obliques whenever possible
	int Vertical;
};

enum uiDrawTextAlign {
	uiDrawTextAlignLeft,
	uiDrawTextAlignCenter,
	uiDrawTextAlignRight,
};

struct uiDrawTextBlockParams {
	intmax_t Width;
	intmax_t FirstLineIndent;
	intmax_t LineSpacing;
	uiDrawTextAlign Align;
	int Justify;
};

struct uiDrawFontMetrics {
	// TODO
	// metrics on Windows are per-device; other platforms not?
};

#define uiDrawTextPixelSizeToPointSize(pix, dpiY) \
	((uintmax_t) ((((double) (pix)) * 72.0) / ((double) (dpiY))))
#define uiDrawTextPointSizeToPixelSize(pt, dpiY) \
	((uintmax_t) (((double) (pt)) * (((double) (dpiY)) / 72.0)))

uiDrawFont *uiDrawPrepareFont(uiDrawFontSpec *);
void uiDrawFreeFont(uiDrawFont *);

void uiDrawText(uiDrawContext *, const char *, uiDrawFont *, intmax_t, intmax_t);
void uiDrawTextBlock(uiDrawContext *, const char *, uiDrawFont *, intmax_t, intmax_t, uiDrawTextBlockParams *);
void uiDrawTextExtents(uiDrawContext *, const char *, uiDrawFont *, intmax_t *, intmax_t *);
intmax_t uiDrawTextExtentsBlockHeight(uiDrawContext *, const char *, uiDrawFont *, uiDrawTextBlockParams *);
// TODO width for number of lines
//TODOvoid uiDrawContextFontMetrics(uiDrawContext *, uiDrawFont *, uiDrawFontMetrics *);

// TODO draw text, single line, control font
// TODO draw text, wrapped to width, control font
// TODO get text extents, single line, control font
// TODO get text height for width, control font
// TODO width for number of lines, control font
// TODO get font metrics, control font

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
