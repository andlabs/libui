typedef struct uiAttributedString uiAttributedString;

_UI_ENUM(uiAttribute) {
	// TODO
};

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

typedef struct uiDrawTextLayout uiDrawTextLayout;
typedef struct uiDrawTextLayoutLineMetrics uiDrawTextLayoutLineMetrics;
typedef struct uiDrawTextLayoutByteRangeRectangle uiDrawTextLayoutByteRangeRectangle;

struct uiDrawTextLayoutLineMetrics {
	double X;
	double Y;
	double Width;
	double Ascent;
	double Descent;
	double Leading;
};

_UI_ENUM(uiDrawTextLayoutHitTestResult) {
	uiDrawTextLayoutHitTestResultNowhere,
	uiDrawTextLayoutHitTestResultOnLineTrailingWhitespace,
	uiDrawTextLayoutHitTestResultOnCharacter,
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

_UI_EXTERN uiDrawTextLayout *uiDrawNewTextLayout(uiAttributedString *s, /* TODO default font */, double width);
_UI_EXTERN void uiDrawFreeTextLayout(uiDrawTextLayout *tl);
_UI_EXTERN void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y);
_UI_EXTERN void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height);
_UI_EXTERN int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl);
_UI_EXTERN void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end);
_UI_EXTERN void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m);
// TODO redo this? remove it entirely?
_UI_EXTERN void uiDrawTextLayoutByteIndexToGraphemeRect(uiDrawTextLayout *tl, size_t pos, int *line, double *x, double *y, double *width, double *height);
// TODO partial offset?
_UI_EXTERN uiDrawTextLayoutHitTestResult uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, size_t *byteIndex, int *line);
_UI_EXTERN void uiDrawTextLayoutByteRangeToRectangle(uiDrawTextLayout *tl, size_t start, size_t end, uiDrawTextLayoutByteRangeRectangle *r);
// TODO draw only a line?
// TODO other layout-specific attributes (alignment, wrapping, etc.)?
// TODO number of lines visible for clipping rect, range visible for clipping rect?
