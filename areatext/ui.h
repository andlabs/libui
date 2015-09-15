// 15 september 2015

// TODO uiDrawBeginTextRGB(/RGBA?)
// might need something more flexible for attributed text

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
