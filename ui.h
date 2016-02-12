// 6 april 2015

// TODO rename all uiXxxType() to uiTypeXxx()?

#ifndef __LIBUI_UI_H__
#define __LIBUI_UI_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO add __declspec(dllimport) on windows
#ifndef _UI_EXTERN
#define _UI_EXTERN extern
#endif

typedef struct uiInitOptions uiInitOptions;

struct uiInitOptions {
	size_t Size;
};

_UI_EXTERN const char *uiInit(uiInitOptions *options);
_UI_EXTERN void uiUninit(void);
_UI_EXTERN void uiFreeInitError(const char *err);

_UI_EXTERN void uiMain(void);
_UI_EXTERN void uiQuit(void);

// TODO write a test for this after adding multiline entries
_UI_EXTERN void uiQueueMain(void (*f)(void *data), void *data);

_UI_EXTERN void uiOnShouldQuit(int (*f)(void *data), void *data);

_UI_EXTERN void uiFreeText(char *text);

_UI_EXTERN uintmax_t uiRegisterType(const char *name, uintmax_t parent, size_t size);
_UI_EXTERN void *uiIsA(void *p, uintmax_t type, int fail);
typedef struct uiTyped uiTyped;
struct uiTyped {
	uintmax_t Type;
};
#define uiTyped(this) ((uiTyped *) (this))

typedef struct uiControl uiControl;

struct uiControl {
	uiTyped t;
	void *Internal;		// for use by libui only
	void (*CommitDestroy)(uiControl *);
	uintptr_t (*Handle)(uiControl *);
	void (*CommitShow)(uiControl *);
	void (*CommitHide)(uiControl *);
	void (*ContainerUpdateState)(uiControl *);
};
_UI_EXTERN uintmax_t uiControlType(void);
#define uiControl(this) ((uiControl *) uiIsA((this), uiControlType(), 1))
_UI_EXTERN void uiControlDestroy(uiControl *);
_UI_EXTERN uintptr_t uiControlHandle(uiControl *);
_UI_EXTERN uiControl *uiControlParent(uiControl *);
_UI_EXTERN void uiControlSetParent(uiControl *, uiControl *);
_UI_EXTERN void uiControlShow(uiControl *);
_UI_EXTERN void uiControlHide(uiControl *);
_UI_EXTERN void uiControlEnable(uiControl *);
_UI_EXTERN void uiControlDisable(uiControl *);
_UI_EXTERN uiControl *uiNewControl(uintmax_t type);

/* TODO
func NewControl(type uintmax_t) *Control;

raw "#define uiDefineControlType(typename, funcname, realtype) \\";
raw "	static uintmax_t type_ ## typename = 0; \\";
raw "	uintmax_t funcname(void) \\";
raw "	{ \\";
raw "		if (type_ ## typename == 0) \\";
raw "			type_ ## typename = uiRegisterType(#typename, uiTypeControl(), sizeof (realtype)); \\";
raw "		return type_ ## typename; \\";
raw "	}";

func FreeSizing(d *Sizing);

func MakeContainer(c *Control) uintptr_t;
*/

typedef struct uiWindow uiWindow;
_UI_EXTERN uintmax_t uiWindowType(void);
#define uiWindow(this) ((uiWindow *) uiIsA((this), uiWindowType(), 1))
_UI_EXTERN char *uiWindowTitle(uiWindow *w);
_UI_EXTERN void uiWindowSetTitle(uiWindow *w, const char *title);
_UI_EXTERN void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *w, void *data), void *data);
_UI_EXTERN void uiWindowSetChild(uiWindow *w, uiControl *child);
_UI_EXTERN int uiWindowMargined(uiWindow *w);
_UI_EXTERN void uiWindowSetMargined(uiWindow *w, int margined);
_UI_EXTERN uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar);

typedef struct uiButton uiButton;
_UI_EXTERN uintmax_t uiButtonType(void);
#define uiButton(this) ((uiButton *) uiIsA((this), uiButtonType(), 1))
_UI_EXTERN char *uiButtonText(uiButton *b);
_UI_EXTERN void uiButtonSetText(uiButton *b, const char *text);
_UI_EXTERN void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *b, void *data), void *data);
_UI_EXTERN uiButton *uiNewButton(const char *text);

typedef struct uiBox uiBox;
_UI_EXTERN uintmax_t uiBoxType(void);
#define uiBox(this) ((uiBox *) uiIsA((this), uiBoxType(), 1))
_UI_EXTERN void uiBoxAppend(uiBox *b, uiControl *child, int stretchy);
_UI_EXTERN void uiBoxDelete(uiBox *b, uintmax_t index);
_UI_EXTERN int uiBoxPadded(uiBox *b);
_UI_EXTERN void uiBoxSetPadded(uiBox *b, int padded);
_UI_EXTERN uiBox *uiNewHorizontalBox(void);
_UI_EXTERN uiBox *uiNewVerticalBox(void);

typedef struct uiEntry uiEntry;
_UI_EXTERN uintmax_t uiEntryType(void);
#define uiEntry(this) ((uiEntry *) uiIsA((this), uiEntryType(), 1))
_UI_EXTERN char *uiEntryText(uiEntry *e);
_UI_EXTERN void uiEntrySetText(uiEntry *e, const char *text);
_UI_EXTERN void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *e, void *data), void *data);
_UI_EXTERN int uiEntryReadOnly(uiEntry *e);
_UI_EXTERN void uiEntrySetReadOnly(uiEntry *e, int readonly);
_UI_EXTERN uiEntry *uiNewEntry(void);

typedef struct uiCheckbox uiCheckbox;
_UI_EXTERN uintmax_t uiCheckboxType(void);
#define uiCheckbox(this) ((uiCheckbox *) uiIsA((this), uiCheckboxType(), 1))
_UI_EXTERN char *uiCheckboxText(uiCheckbox *c);
_UI_EXTERN void uiCheckboxSetText(uiCheckbox *c, const char *text);
_UI_EXTERN void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *c, void *data), void *data);
_UI_EXTERN int uiCheckboxChecked(uiCheckbox *c);
_UI_EXTERN void uiCheckboxSetChecked(uiCheckbox *c, int checked);
_UI_EXTERN uiCheckbox *uiNewCheckbox(const char *text);

typedef struct uiLabel uiLabel;
_UI_EXTERN uintmax_t uiLabelType(void);
#define uiLabel(this) ((uiLabel *) uiIsA((this), uiLabelType(), 1))
_UI_EXTERN char *uiLabelText(uiLabel *l);
_UI_EXTERN void uiLabelSetText(uiLabel *l, const char *text);
_UI_EXTERN uiLabel *uiNewLabel(const char *text);

typedef struct uiTab uiTab;
_UI_EXTERN uintmax_t uiTabType(void);
#define uiTab(this) ((uiTab *) uiIsA((this), uiTabType(), 1))
_UI_EXTERN void uiTabAppend(uiTab *t, const char *name, uiControl *c);
_UI_EXTERN void uiTabInsertAt(uiTab *t, const char *name, uintmax_t before, uiControl *c);
_UI_EXTERN void uiTabDelete(uiTab *t, uintmax_t index);
_UI_EXTERN uintmax_t uiTabNumPages(uiTab *t);
_UI_EXTERN int uiTabMargined(uiTab *t, uintmax_t page);
_UI_EXTERN void uiTabSetMargined(uiTab *t, uintmax_t page, int margined);
_UI_EXTERN uiTab *uiNewTab(void);

typedef struct uiGroup uiGroup;
_UI_EXTERN uintmax_t uiGroupType(void);
#define uiGroup(this) ((uiGroup *) uiIsA((this), uiGroupType(), 1))
_UI_EXTERN char *uiGroupTitle(uiGroup *g);
_UI_EXTERN void uiGroupSetTitle(uiGroup *g, const char *title);
_UI_EXTERN void uiGroupSetChild(uiGroup *g, uiControl *c);
_UI_EXTERN int uiGroupMargined(uiGroup *g);
_UI_EXTERN void uiGroupSetMargined(uiGroup *g, int margined);
_UI_EXTERN uiGroup *uiNewGroup(const char *title);

// spinbox/slider rules:
// setting value outside of range will automatically clamp
// initial value is minimum
// complaint if min >= max?

typedef struct uiSpinbox uiSpinbox;
_UI_EXTERN uintmax_t uiSpinboxType(void);
#define uiSpinbox(this) ((uiSpinbox *) uiIsA((this), uiSpinboxType(), 1))
_UI_EXTERN intmax_t uiSpinboxValue(uiSpinbox *s);
_UI_EXTERN void uiSpinboxSetValue(uiSpinbox *s, intmax_t value);
_UI_EXTERN void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *s, void *data), void *data);
_UI_EXTERN uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max);

typedef struct uiProgressBar uiProgressBar;
_UI_EXTERN uintmax_t uiProgressBarType(void);
#define uiProgressBar(this) ((uiProgressBar *) uiIsA((this), uiProgressBarType(), 1))
// TODO uiProgressBarValue()
_UI_EXTERN void uiProgressBarSetValue(uiProgressBar *p, int n);
_UI_EXTERN uiProgressBar *uiNewProgressBar(void);

typedef struct uiSlider uiSlider;
_UI_EXTERN uintmax_t uiSliderType(void);
#define uiSlider(this) ((uiSlider *) uiIsA((this), uiSliderType(), 1))
_UI_EXTERN intmax_t uiSliderValue(uiSlider *s);
_UI_EXTERN void uiSliderSetValue(uiSlider *s, intmax_t value);
_UI_EXTERN void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *s, void *data), void *data);
_UI_EXTERN uiSlider *uiNewSlider(intmax_t min, intmax_t max);

typedef struct uiSeparator uiSeparator;
_UI_EXTERN uintmax_t uiSeparatorType(void);
#define uiSeparator(this) ((uiSeparator *) uiIsA((this), uiSeparatorType(), 1))
_UI_EXTERN uiSeparator *uiNewHorizontalSeparator(void);

typedef struct uiCombobox uiCombobox;
_UI_EXTERN uintmax_t uiComboboxType(void);
#define uiCombobox(this) ((uiCombobox *) uiIsA((this), uiComboboxType(), 1))
_UI_EXTERN void uiComboboxAppend(uiCombobox *c, const char *text);
_UI_EXTERN intmax_t uiComboboxSelected(uiCombobox *c);
_UI_EXTERN void uiComboboxSetSelected(uiCombobox *c, intmax_t n);
_UI_EXTERN void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data);
_UI_EXTERN uiCombobox *uiNewCombobox(void);
_UI_EXTERN uiCombobox *uiNewEditableCombobox(void);

typedef struct uiRadioButtons uiRadioButtons;
_UI_EXTERN uintmax_t uiRadioButtonsType(void);
#define uiRadioButtons(this) ((uiRadioButtons *) uiIsA((this), uiRadioButtonsType(), 1))
_UI_EXTERN void uiRadioButtonsAppend(uiRadioButtons *r, const char *text);
_UI_EXTERN uiRadioButtons *uiNewRadioButtons(void);

typedef struct uiDateTimePicker uiDateTimePicker;
_UI_EXTERN uintmax_t uiDateTimePickerType(void);
#define uiDateTimePicker(this) ((uiDateTimePicker *) uiIsA((this), uiDateTimePickerType(), 1))
_UI_EXTERN uiDateTimePicker *uiNewDateTimePicker(void);
_UI_EXTERN uiDateTimePicker *uiNewDatePicker(void);
_UI_EXTERN uiDateTimePicker *uiNewTimePicker(void);

// TODO merge with uiEntry? some things can't be shared (for instance, the future Invalid()
// TODO how are line endings converted?
// TODO provide a facility for allowing horizontal scrolling
// TODO provide a facility for entering tab stops?
typedef struct uiMultilineEntry uiMultilineEntry;
_UI_EXTERN uintmax_t uiMultilineEntryType(void);
#define uiMultilineEntry(this) ((uiMultilineEntry *) uiIsA((this), uiMultilineEntryType(), 1))
_UI_EXTERN char *uiMultilineEntryText(uiMultilineEntry *e);
_UI_EXTERN void uiMultilineEntrySetText(uiMultilineEntry *e, const char *text);
_UI_EXTERN void uiMultilineEntryAppend(uiMultilineEntry *e, const char *text);
_UI_EXTERN void uiMultilineEntryOnChanged(uiMultilineEntry *e, void (*f)(uiMultilineEntry *e, void *data), void *data);
_UI_EXTERN int uiMultilineEntryReadOnly(uiMultilineEntry *e);
_UI_EXTERN void uiMultilineEntrySetReadOnly(uiMultilineEntry *e, int readonly);
_UI_EXTERN uiMultilineEntry *uiNewMultilineEntry(void);

typedef struct uiMenu uiMenu;
typedef struct uiMenuItem uiMenuItem;
_UI_EXTERN uintmax_t uiMenuType(void);
#define uiMenu(this) ((uiMenu *) uiIsA((this), uiMenuType(), 1))
_UI_EXTERN uiMenuItem *uiMenuAppendItem(uiMenu *m, const char *name);
_UI_EXTERN uiMenuItem *uiMenuAppendCheckItem(uiMenu *m, const char *name);
_UI_EXTERN uiMenuItem *uiMenuAppendQuitItem(uiMenu *m);
_UI_EXTERN uiMenuItem *uiMenuAppendPreferencesItem(uiMenu *m);
_UI_EXTERN uiMenuItem *uiMenuAppendAboutItem(uiMenu *m);
_UI_EXTERN void uiMenuAppendSeparator(uiMenu *m);
_UI_EXTERN uiMenu *uiNewMenu(const char *name);

_UI_EXTERN uintmax_t uiMenuItemType(void);
#define uiMenuItem(this) ((uiMenuItem *) uiIsA((this), uiMenuItemType(), 1))
_UI_EXTERN void uiMenuItemEnable(uiMenuItem *m);
_UI_EXTERN void uiMenuItemDisable(uiMenuItem *m);
_UI_EXTERN void uiMenuItemOnClicked(uiMenuItem *m, void (*f)(uiMenuItem *sender, uiWindow *window, void *data), void *data);
_UI_EXTERN int uiMenuItemChecked(uiMenuItem *m);
_UI_EXTERN void uiMenuItemSetChecked(uiMenuItem *m, int checked);

_UI_EXTERN char *uiOpenFile(uiWindow *parent);
_UI_EXTERN char *uiSaveFile(uiWindow *parent);
_UI_EXTERN void uiMsgBox(uiWindow *parent, const char *title, const char *description);
_UI_EXTERN void uiMsgBoxError(uiWindow *parent, const char *title, const char *description);

typedef struct uiArea uiArea;
typedef struct uiAreaHandler uiAreaHandler;
typedef struct uiAreaDrawParams uiAreaDrawParams;
typedef struct uiAreaMouseEvent uiAreaMouseEvent;
typedef struct uiAreaKeyEvent uiAreaKeyEvent;

typedef struct uiDrawContext uiDrawContext;

struct uiAreaHandler {
	void (*Draw)(uiAreaHandler *, uiArea *, uiAreaDrawParams *);
	// TODO document that resizes cause a full redraw for non-scrolling areas; implementation-defined for scrolling areas
	void (*MouseEvent)(uiAreaHandler *, uiArea *, uiAreaMouseEvent *);
	// TODO document that on first show if the mouse is already in the uiArea then one gets sent with left=0
	// TODO what about when the area is hidden and then shown again?
	void (*MouseCrossed)(uiAreaHandler *, uiArea *, int left);
	void (*DragBroken)(uiAreaHandler *, uiArea *);
	int (*KeyEvent)(uiAreaHandler *, uiArea *, uiAreaKeyEvent *);
};

_UI_EXTERN uintmax_t uiMenuItemType(void);
#define uiArea(this) ((uiArea *) uiIsA((this), uiAreaType(), 1))
// TODO give a better name
// TODO document the types of width and height
_UI_EXTERN void uiAreaSetSize(uiArea *a, intmax_t width, intmax_t height);
// TODO uiAreaQueueRedraw()
_UI_EXTERN void uiAreaQueueRedrawAll(uiArea *a);
_UI_EXTERN uiArea *uiNewArea(uiAreaHandler *ah);
_UI_EXTERN uiArea *uiNewScrollingArea(uiAreaHandler *ah, intmax_t width, intmax_t height);

struct uiAreaDrawParams {
	uiDrawContext *Context;

	// TODO document that this is only defined for nonscrolling areas
	double AreaWidth;
	double AreaHeight;

	// TODO keep this?
	double ClipX;
	double ClipY;
	double ClipWidth;
	double ClipHeight;
};

typedef struct uiDrawPath uiDrawPath;
typedef struct uiDrawBrush uiDrawBrush;
typedef struct uiDrawStrokeParams uiDrawStrokeParams;
typedef struct uiDrawMatrix uiDrawMatrix;

typedef struct uiDrawBrushGradientStop uiDrawBrushGradientStop;

// ISO C forbids us from forward declaring enums >:(
typedef enum uiDrawBrushType {
	uiDrawBrushTypeSolid,
	uiDrawBrushTypeLinearGradient,
	uiDrawBrushTypeRadialGradient,
	uiDrawBrushTypeImage,
} uiDrawBrushType;

typedef enum uiDrawLineCap {
	uiDrawLineCapFlat,
	uiDrawLineCapRound,
	uiDrawLineCapSquare,
} uiDrawLineCap;

typedef enum uiDrawLineJoin {
	uiDrawLineJoinMiter,
	uiDrawLineJoinRound,
	uiDrawLineJoinBevel,
} uiDrawLineJoin;

// this is the default for botoh cairo and Direct2D (in the latter case, from the C++ helper functions)
// Core Graphics doesn't explicitly specify a default, but NSBezierPath allows you to choose one, and this is the initial value
// so we're good to use it too!
#define uiDrawDefaultMiterLimit 10.0

typedef enum uiDrawFillMode {
	uiDrawFillModeWinding,
	uiDrawFillModeAlternate,
} uiDrawFillMode;

struct uiDrawMatrix {
	double M11;
	double M12;
	double M21;
	double M22;
	double M31;
	double M32;
};

struct uiDrawBrush {
	uiDrawBrushType Type;

	// solid brushes
	double R;
	double G;
	double B;
	double A;

	// gradient brushes
	double X0;		// linear: start X, radial: start X
	double Y0;		// linear: start Y, radial: start Y
	double X1;		// linear: end X, radial: outer circle center X
	double Y1;		// linear: end Y, radial: outer circle center Y
	double OuterRadius;		// radial gradients only
	uiDrawBrushGradientStop *Stops;
	size_t NumStops;
	// TODO extend mode
	// cairo: none, repeat, reflect, pad; no individual control
	// Direct2D: repeat, reflect, pad; no individual control
	// Core Graphics: none, pad; before and after individually
	// TODO cairo documentation is inconsistent about pad

	// TODO images

	// TODO transforms
};

struct uiDrawBrushGradientStop {
	double Pos;
	double R;
	double G;
	double B;
	double A;
};

struct uiDrawStrokeParams {
	uiDrawLineCap Cap;
	uiDrawLineJoin Join;
	// TODO what if this is 0? on windows there will be a crash with dashing
	double Thickness;
	double MiterLimit;
	double *Dashes;
	// TOOD what if this is 1 on Direct2D?
	// TODO what if a dash is 0 on Cairo or Quartz?
	size_t NumDashes;
	double DashPhase;
};

_UI_EXTERN uiDrawPath *uiDrawNewPath(uiDrawFillMode fillMode);
_UI_EXTERN void uiDrawFreePath(uiDrawPath *p);

_UI_EXTERN void uiDrawPathNewFigure(uiDrawPath *p, double x, double y);
_UI_EXTERN void uiDrawPathNewFigureWithArc(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative);
_UI_EXTERN void uiDrawPathLineTo(uiDrawPath *p, double x, double y);
// notes: angles are both relative to 0 and go counterclockwise
// TODO is the initial line segment on cairo and OS X a proper join?
// TODO what if sweep < 0?
_UI_EXTERN void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative);
_UI_EXTERN void uiDrawPathBezierTo(uiDrawPath *p, double c1x, double c1y, double c2x, double c2y, double endX, double endY);
// TODO quadratic bezier
_UI_EXTERN void uiDrawPathCloseFigure(uiDrawPath *p);

// TODO effect of these when a figure is already started
_UI_EXTERN void uiDrawPathAddRectangle(uiDrawPath *p, double x, double y, double width, double height);

_UI_EXTERN void uiDrawPathEnd(uiDrawPath *p);

_UI_EXTERN void uiDrawStroke(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b, uiDrawStrokeParams *p);
_UI_EXTERN void uiDrawFill(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b);

// TODO primitives:
// - rounded rectangles
// - elliptical arcs
// - quadratic bezier curves

_UI_EXTERN void uiDrawMatrixSetIdentity(uiDrawMatrix *m);
_UI_EXTERN void uiDrawMatrixTranslate(uiDrawMatrix *m, double x, double y);
_UI_EXTERN void uiDrawMatrixScale(uiDrawMatrix *m, double xCenter, double yCenter, double x, double y);
_UI_EXTERN void uiDrawMatrixRotate(uiDrawMatrix *m, double x, double y, double amount);
_UI_EXTERN void uiDrawMatrixSkew(uiDrawMatrix *m, double x, double y, double xamount, double yamount);
_UI_EXTERN void uiDrawMatrixMultiply(uiDrawMatrix *dest, uiDrawMatrix *src);
_UI_EXTERN int uiDrawMatrixInvertible(uiDrawMatrix *m);
_UI_EXTERN int uiDrawMatrixInvert(uiDrawMatrix *m);
_UI_EXTERN void uiDrawMatrixTransformPoint(uiDrawMatrix *m, double *x, double *y);
_UI_EXTERN void uiDrawMatrixTransformSize(uiDrawMatrix *m, double *x, double *y);

_UI_EXTERN void uiDrawTransform(uiDrawContext *c, uiDrawMatrix *m);

// TODO add a uiDrawPathStrokeToFill() or something like that
_UI_EXTERN void uiDrawClip(uiDrawContext *c, uiDrawPath *path);

_UI_EXTERN void uiDrawSave(uiDrawContext *c);
_UI_EXTERN void uiDrawRestore(uiDrawContext *c);

// TODO manage the use of Text, Font, and TextFont, and of the uiDrawText prefix in general

///// TODO
typedef struct uiDrawFontFamilies uiDrawFontFamilies;

_UI_EXTERN uiDrawFontFamilies *uiDrawListFontFamilies(void);
_UI_EXTERN uintmax_t uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff);
_UI_EXTERN char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, uintmax_t n);
_UI_EXTERN void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff);
///// END TODO

typedef struct uiDrawTextLayout uiDrawTextLayout;
typedef struct uiDrawTextFont uiDrawTextFont;
typedef struct uiDrawTextFontDescriptor uiDrawTextFontDescriptor;
typedef struct uiDrawTextFontMetrics uiDrawTextFontMetrics;

typedef enum uiDrawTextWeight {
	uiDrawTextWeightThin,
	uiDrawTextWeightUltraLight,
	uiDrawTextWeightLight,
	uiDrawTextWeightBook,
	uiDrawTextWeightNormal,
	uiDrawTextWeightMedium,
	uiDrawTextWeightSemiBold,
	uiDrawTextWeightBold,
	uiDrawTextWeightUtraBold,
	uiDrawTextWeightHeavy,
	uiDrawTextWeightUltraHeavy,
} uiDrawTextWeight;

// TODO drop Oblique?
typedef enum uiDrawTextItalic {
	uiDrawTextItalicNormal,
	uiDrawTextItalicOblique,
	uiDrawTextItalicItalic,
} uiDrawTextItalic;

typedef enum uiDrawTextStretch {
	uiDrawTextStretchUltraCondensed,
	uiDrawTextStretchExtraCondensed,
	uiDrawTextStretchCondensed,
	uiDrawTextStretchSemiCondensed,
	uiDrawTextStretchNormal,
	uiDrawTextStretchSemiExpanded,
	uiDrawTextStretchExpanded,
	uiDrawTextStretchExtraExpanded,
	uiDrawTextStretchUltraExpanded,
} uiDrawTextStretch;

typedef enum uiDrawTextGravity {
	uiDrawTextGravitySouth,
	uiDrawTextGravityEast,
	uiDrawTextGravityNorth,
	uiDrawTextGravityWest,
	uiDrawTextGravityAuto,
} uiDrawTextGravity;

struct uiDrawTextFontDescriptor {
	const char *Family;
	double Size;
	uiDrawTextWeight Weight;
	uiDrawTextItalic Italic;
	int SmallCaps;
	uiDrawTextStretch Stretch;
	uiDrawTextGravity Gravity;
};

struct uiDrawTextFontMetrics {
	double Ascent;
	double Descent;
	double Leading;
	// TODO do these two mean the same across all platforms?
	double UnderlinePos;
	double UnderlineThickness;
};

_UI_EXTERN uiDrawTextFont *uiDrawLoadClosestFont(const uiDrawTextFontDescriptor *desc);
_UI_EXTERN void uiDrawFreeTextFont(uiDrawTextFont *font);
_UI_EXTERN uintptr_t uiDrawTextFontHandle(uiDrawTextFont *font);
_UI_EXTERN void uiDrawTextFontDescribe(uiDrawTextFont *font, uiDrawTextFontDescriptor *desc);
// TODO make copy with given attributes methods?
// TODO yuck this name
_UI_EXTERN void uiDrawTextFontGetMetrics(uiDrawTextFont *font, uiDrawTextFontMetrics *metrics);

// TODO initial line spacing? and what about leading?
_UI_EXTERN uiDrawTextLayout *uiDrawNewTextLayout(const char *text, uiDrawTextFont *defaultFont, double width);
_UI_EXTERN void uiDrawFreeTextLayout(uiDrawTextLayout *layout);
// TODO get width
_UI_EXTERN void uiDrawTextLayoutSetWidth(uiDrawTextLayout *layout, double width);
_UI_EXTERN void uiDrawTextLayoutExtents(uiDrawTextLayout *layout, double *width, double *height);

_UI_EXTERN void uiDrawText(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout);

typedef enum uiModifiers {
	uiModifierCtrl = 1 << 0,
	uiModifierAlt = 1 << 1,
	uiModifierShift = 1 << 2,
	uiModifierSuper = 1 << 3,
} uiModifiers;

// TODO document drag captures
struct uiAreaMouseEvent {
	// TODO document what these mean for scrolling areas
	double X;
	double Y;

	// TODO see draw above
	double AreaWidth;
	double AreaHeight;

	uintmax_t Down;
	uintmax_t Up;

	uintmax_t Count;

	uiModifiers Modifiers;

	uint64_t Held1To64;
};

typedef enum uiExtKey {
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
} uiExtKey;

struct uiAreaKeyEvent {
	char Key;
	uiExtKey ExtKey;
	uiModifiers Modifier;

	uiModifiers Modifiers;

	int Up;
};

#ifdef __cplusplus
}
#endif

#endif
