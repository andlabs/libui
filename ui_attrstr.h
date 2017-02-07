typedef struct uiAttributedString uiAttributedString;

_UI_ENUM(uiAttribute) {
	// TODO uiAttributeFamily,
	// TODO uiAttributeSize,
	uiAttributeWeight,
	// TODO
	// TODO uiAttributeSystem,
	// TODO uiAttributeCustom,
};

typedef int (*uiAttributedStringForEachAttributeFunc)(uiAttributedString *, uiAttribute type, uintptr_t value, size_t start, size_t end, void *data);

// @role uiAttributedString constructor
// uiNewAttributedString() creates a new uiAttributedString from
// initialString. The string will be entirely unattributed.
_UI_EXTERN uiAttributedString *uiNewAttributedString(const char *initialString);

// @role uiAttributedString destructor
// uiFreeAttributedString() destroys the uiAttributedString s.
_UI_EXTERN void uiFreeAttributedString(uiAttributedString *s);

// uiAttributedStringString() returns the textual content of s as a
// '\0'-terminated UTF-8 string. The returned pointer is valid until
// the next change to the textual content of s.
_UI_EXTERN const char *uiAttributedStringString(uiAttributedString *s);

// uiAttributedStringLength() returns the number of UTF-8 bytes in
// the textual content of s, excluding the terminating '\0'.
_UI_EXTERN size_t uiAttributedStringLen(uiAttributedString *s);

_UI_EXTERN void uiAttributedStringAppendUnattributed(uiAttributedString *s, const char *str);
_UI_EXTERN void uiAttributedStringInsertAtUnattributed(uiAttributedString *s, const char *str, size_t at);
_UI_EXTERN void uiAttributedStringDelete(uiAttributedString *s, size_t start, size_t end);
_UI_EXTERN size_t uiAttributedStringNumGraphemes(uiAttributedString *s);
_UI_EXTERN size_t uiAttributedStringByteIndexToGrapheme(uiAttributedString *s, size_t pos);
_UI_EXTERN size_t uiAttributedStringGraphemeToByteIndex(uiAttributedString *s, size_t pos);
_UI_EXTERN void uiAttributedStringForEachAttribute(uiAttributedString *s, uiAttributedStringForEachAttributeFunc f, void *data);

typedef struct uiDrawFontDescriptor uiDrawFontDescriptor;

// TODO Minimum == 1? IIRC there is at least one font on OS X that actually has a weight of 0
// TODO Maximum == 999? IIRC there is at least one font on OS X that actually has a weight of 1000
_UI_ENUM(uiDrawTextWeight) {
	uiDrawTextWeightMinimum = 0,
	uiDrawTextWeightThin = 100,
	uiDrawTextWeightUltraLight = 200,
	uiDrawTextWeightLight = 300,
	uiDrawTextWeightBook = 350,
	uiDrawTextWeightNormal = 400,
	uiDrawTextWeightMedium = 500,
	uiDrawTextWeightSemiBold = 600,
	uiDrawTextWeightBold = 700,
	uiDrawTextWeightUltraBold = 800,
	uiDrawTextWeightHeavy = 900,
	uiDrawTextWeightUltraHeavy = 950,
	uiDrawTextWeightMaximum = 1000,
};

_UI_ENUM(uiDrawTextItalic) {
	uiDrawTextItalicNormal,
	uiDrawTextItalicOblique,
	uiDrawTextItalicItalic,
};

// TODO realign this  so that Normal == 0?
_UI_ENUM(uiDrawTextStretch) {
	uiDrawTextStretchUltraCondensed,
	uiDrawTextStretchExtraCondensed,
	uiDrawTextStretchCondensed,
	uiDrawTextStretchSemiCondensed,
	uiDrawTextStretchNormal,
	uiDrawTextStretchSemiExpanded,
	uiDrawTextStretchExpanded,
	uiDrawTextStretchExtraExpanded,
	uiDrawTextStretchUltraExpanded,
};

struct uiDrawFontDescriptor {
	char *Family;
	// TODO rename to PointSize?
	double Size;
	uiDrawTextWeight Weight;
	uiDrawTextItalic Italic;
	uiDrawTextStretch Stretch;
};

typedef struct uiDrawTextLayout uiDrawTextLayout;
typedef struct uiDrawTextLayoutLineMetrics uiDrawTextLayoutLineMetrics;
typedef struct uiDrawTextLayoutHitTestResult uiDrawTextLayoutHitTestResult;
typedef struct uiDrawTextLayoutByteRangeRectangle uiDrawTextLayoutByteRangeRectangle;

struct uiDrawTextLayoutLineMetrics {
	// This describes the overall bounding box of the line.
	// TODO figure out if X is correct regardless of both alignment and writing direction
	double X;
	double Y;
	double Width;
	double Height;

	// This describes the typographic bounds of the line.
	double BaselineY;
	double Ascent;
	double Descent;
	double Leading;

	// This describes any additional whitespace.
	// TODO come up with better names for these.
	double ParagraphSpacingBefore;
	double LineHeightSpace;
	double LineSpacing;
	double ParagraphSpacing;

	// Height should equal ParagraphSpacingBefore + LineHeightSpace + Ascent + Descent + Leading + LineSpacing + ParagraphSpacing.
	// The above values are listed in vertical order, from top to bottom.
	// Ascent + Descent + Leading will give you the typographic bounds of the text.
	// BaselineY will be the boundary between Ascent and Descent.
	// X, Y, and BaselineY are all in the layout's coordinate system, so the start point of the baseline will be at (X, BaselineY).
	// All values will be nonnegative.

	// TODO trailing whitespace?
};

_UI_ENUM(uiDrawTextLayoutHitTestPosition) {
	uiDrawTextLayoutHitTestPositionBefore,
	uiDrawTextLayoutHitTestPositionInside,
	uiDrawTextLayoutHitTestPositionAfter,
};

struct uiDrawTextLayoutHitTestResult {
	// The byte position of the character at the given point.
	size_t Pos;
	// Line is the line at the given point. If the point is on the space
	// after the end of a line, Pos will be Line's end index. In this case,
	// the rectangle for that character will actually be on the *next*
	// line. This disparity is only relevant for caret positioning when
	// using the mouse; in that case, to ensure proper behavior, use
	// the Caret fields below. In all other cases (including keyboard
	// caret movement), use the actual rectangle for the grapheme
	// at Pos (via uiDrawTextLayoutByteRangeToRectangle()).
	int Line;
	uiDrawTextLayoutHitTestPosition XPosition;
	uiDrawTextLayoutHitTestPosition YPosition;

	double CaretX;
	double CaretY;
	// CaretWidth is decided by uiDrawCaret().
	double CaretHeight;
// TODO?
//	int InTrailingWhitespace;
// TODO?
//	double XFraction;
// extra TODO?
//	double YFraction;
// or just have offsets instead? in addition?
};

struct uiDrawTextLayoutByteRangeRectangle {
	int Line;
	double X;
	double Y;
	double Width;
	double Height;
	size_t RealStart;
	size_t RealEnd;
};

// TODO
// - allow creating a layout out of a substring
// - allow marking compositon strings
// - allow marking selections, even after creation
// - add the following functions:
// 	- uiDrawTextLayoutHeightForWidth() (returns the height that a layout would need to be to display the entire string at a given width)
// 	- uiDrawTextLayoutRangeForSize() (returns what substring would fit in a given size)
// 	- uiDrawTextLayoutNewWithHeight() (limits amount of string used by the height)
_UI_EXTERN uiDrawTextLayout *uiDrawNewTextLayout(uiAttributedString *s, uiDrawFontDescriptor *defaultFont, double width);
_UI_EXTERN void uiDrawFreeTextLayout(uiDrawTextLayout *tl);
_UI_EXTERN void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y);
_UI_EXTERN void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height);
_UI_EXTERN int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl);
_UI_EXTERN void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end);
_UI_EXTERN void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m);
_UI_EXTERN void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, uiDrawTextLayoutHitTestResult *result);
_UI_EXTERN void uiDrawTextLayoutByteRangeToRectangle(uiDrawTextLayout *tl, size_t start, size_t end, uiDrawTextLayoutByteRangeRectangle *r);
// TODO draw only a line?
// TODO other layout-specific attributes (alignment, wrapping, etc.)?
// TODO number of lines visible for clipping rect, range visible for clipping rect?
