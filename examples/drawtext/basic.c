// 17 january 2017
#include "drawtext.h"

static const char *text =
	"It is with a kind of fear that I begin to write the history of my life. "
	"I have, as it were, a superstitious hesitation in lifting the veil that "
	"clings about my childhood like a golden mist. The task of writing an "
	"autobiography is a difficult one. When I try to classify my earliest "
	"impressions, I find that fact and fancy look alike across the years that "
	"link the past with the present. The woman paints the child's experiences "
	"in her own fantasy. A few impressions stand out vividly from the first "
	"years of my life; but \"the shadows of the prison-house are on the rest.\" "
	"Besides, many of the joys and sorrows of childhood have lost their "
	"poignancy; and many incidents of vital importance in my early education "
	"have been forgotten in the excitement of great discoveries. In order, "
	"therefore, not to be tedious I shall try to present in a series of "
	"sketches only the episodes that seem to me to be the most interesting "
	"and important."
	"";
static char fontFamily[] = "Palatino";
// TODO should be const; look at constructor function
static uiDrawFontDescriptor defaultFont = {
	.Family = fontFamily,
	.Size = 18,
	.Weight = uiDrawTextWeightNormal,
	.Italic = uiDrawTextItalicNormal,
	.Stretch = uiDrawTextStretchNormal,
};
static uiAttributedString *attrstr;

#define margins 10

static void draw(uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawTextLayout *layout;

	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, margins, margins,
		p->AreaWidth - 2 * margins,
		p->AreaHeight - 2 * margins);
	uiDrawPathEnd(path);
	uiDrawClip(p->Context, path);
	uiDrawFreePath(path);

	// TODO get rid of this later
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, -100, -100,
		p->AreaWidth * 2,
		p->AreaHeight * 2);
	uiDrawPathEnd(path);
	uiDrawBrush b;
	b.Type = uiDrawBrushTypeSolid;
	b.R = 0.0;
	b.G = 1.0;
	b.B = 0.0;
	b.A = 1.0;
	uiDrawFill(p->Context, path, &b);
	uiDrawFreePath(path);

	layout = uiDrawNewTextLayout(attrstr,
		&defaultFont,
		p->AreaWidth - 2 * margins);
	uiDrawText(p->Context, layout, margins, margins);
	uiDrawFreeTextLayout(layout);
}

static struct example basicExample;

struct example *mkBasicExample(void)
{
	basicExample.name = "Basic Paragraph of Text";
	basicExample.panel = uiControl(uiNewVerticalBox());
	basicExample.draw = draw;

	attrstr = uiNewAttributedString(text);

	return &basicExample;
}

// TODO on GTK+ an area by itself in a window doesn't get expand properties set properly?
