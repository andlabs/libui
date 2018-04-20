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
// TODO should be const; look at constructor function?
static uiDrawFontDescriptor defaultFont = {
	.Family = fontFamily,
	.Size = 18,
	.Weight = uiDrawTextWeightNormal,
	.Italic = uiDrawTextItalicNormal,
	.Stretch = uiDrawTextStretchNormal,
};
static uiAttributedString *attrstr;
static uiDrawTextLayoutParams params;

#define margins 10

static uiBox *panel;
static uiCheckbox *showExtents;
static uiCheckbox *showLineBounds;
static uiCheckbox *showLineGuides;

// TODO should this be const?
static double strokeDashes[] = { 5, 2 };
// TODO this should be const
static uiDrawStrokeParams strokeParams = {
	.Cap = uiDrawLineCapFlat,
	.Join = uiDrawLineJoinMiter,
	.Thickness = 1,
	.MiterLimit = uiDrawDefaultMiterLimit,
	.Dashes = strokeDashes,
	.NumDashes = 2,
	.DashPhase = 0,
};

// TODO should be const?
static uiDrawBrush fillBrushes[4] = {
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 1.0,
		.G = 0.0,
		.B = 0.0,
		.A = 0.5,
	},
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.0,
		.G = 1.0,
		.B = 0.0,
		.A = 0.5,
	},
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.0,
		.G = 0.0,
		.B = 1.0,
		.A = 0.5,
	},
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.0,
		.G = 1.0,
		.B = 1.0,
		.A = 0.5,
	},
};
// TODO come up with better colors
static uiDrawBrush strokeBrushes[3] = {
	// baseline
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.5,
		.G = 0.5,
		.B = 0.0,
		.A = 0.75,
	},
	// ascent
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 1.0,
		.G = 0.0,
		.B = 1.0,
		.A = 0.75,
	},
	// descent
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.5,
		.G = 0.75,
		.B = 1.0,
		.A = 0.75,
	},
};

static void draw(uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawTextLayout *layout;
	uiDrawBrush b;

	b.Type = uiDrawBrushTypeSolid;

	// only clip the text, not the guides
	uiDrawSave(p->Context);

	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, margins, margins,
		p->AreaWidth - 2 * margins,
		p->AreaHeight - 2 * margins);
	uiDrawPathEnd(path);
	uiDrawClip(p->Context, path);
	uiDrawFreePath(path);

	// TODO get rid of this later
#if 0
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, -100, -100,
		p->AreaWidth * 2,
		p->AreaHeight * 2);
	uiDrawPathEnd(path);
	b.R = 0.0;
	b.G = 1.0;
	b.B = 0.0;
	b.A = 1.0;
	uiDrawFill(p->Context, path, &b);
	uiDrawFreePath(path);
#endif

	params.Width = p->AreaWidth - 2 * margins;
	layout = uiDrawNewTextLayout(&params);
	uiDrawText(p->Context, layout, margins, margins);

	uiDrawRestore(p->Context);

	if (uiCheckboxChecked(showExtents)) {
		double width, height;

		uiDrawTextLayoutExtents(layout, &width, &height);
		path = uiDrawNewPath(uiDrawFillModeWinding);
		uiDrawPathAddRectangle(path, margins, margins,
			width, height);
		uiDrawPathEnd(path);
		b.R = 1.0;
		b.G = 0.0;
		b.B = 1.0;
		b.A = 0.5;
		uiDrawStroke(p->Context, path, &b, &strokeParams);
		uiDrawFreePath(path);
	}

	if (uiCheckboxChecked(showLineBounds) || uiCheckboxChecked(showLineGuides)) {
		uiDrawTextLayoutLineMetrics m;
		int i, n;
		int fill = 0;

		n = uiDrawTextLayoutNumLines(layout);
		for (i = 0; i < n; i++) {
			uiDrawTextLayoutLineGetMetrics(layout, i, &m);

			if (uiCheckboxChecked(showLineBounds)) {
				path = uiDrawNewPath(uiDrawFillModeWinding);
				uiDrawPathAddRectangle(path, margins + m.X, margins + m.Y,
					m.Width, m.Height);
				uiDrawPathEnd(path);
				uiDrawFill(p->Context, path, fillBrushes + fill);
				uiDrawFreePath(path);
				fill = (fill + 1) % 4;
			}
			if (uiCheckboxChecked(showLineGuides)) {
				// baseline
				path = uiDrawNewPath(uiDrawFillModeWinding);
				uiDrawPathNewFigure(path,
					margins + m.X,
					margins + m.BaselineY);
				uiDrawPathLineTo(path,
					margins + m.X + m.Width,
					margins + m.BaselineY);
				uiDrawPathEnd(path);
				uiDrawStroke(p->Context, path, &(strokeBrushes[0]), &strokeParams);
				uiDrawFreePath(path);

				// ascent line
				path = uiDrawNewPath(uiDrawFillModeWinding);
				uiDrawPathNewFigure(path,
					margins + m.X,
					margins + m.BaselineY - m.Ascent);
				uiDrawPathLineTo(path,
					margins + m.X + m.Width,
					margins + m.BaselineY - m.Ascent);
				uiDrawPathEnd(path);
				uiDrawStroke(p->Context, path, &(strokeBrushes[1]), &strokeParams);
				uiDrawFreePath(path);

				// descent line
				path = uiDrawNewPath(uiDrawFillModeWinding);
				uiDrawPathNewFigure(path,
					margins + m.X,
					margins + m.BaselineY + m.Descent);
				uiDrawPathLineTo(path,
					margins + m.X + m.Width,
					margins + m.BaselineY + m.Descent);
				uiDrawPathEnd(path);
				uiDrawStroke(p->Context, path, &(strokeBrushes[2]), &strokeParams);
				uiDrawFreePath(path);
			}
		}
	}

	uiDrawFreeTextLayout(layout);
}

static struct example basicExample;

// TODO share?
static void checkboxChecked(uiCheckbox *c, void *data)
{
	redraw();
}

static uiCheckbox *newCheckbox(const char *text)
{
	uiCheckbox *c;

	c = uiNewCheckbox(text);
	uiCheckboxOnToggled(c, checkboxChecked, NULL);
	uiBoxAppend(panel, uiControl(c), 0);
	return c;
}

struct example *mkBasicExample(void)
{
	panel = uiNewVerticalBox();
	showExtents = newCheckbox("Show Layout Extents");
	showLineBounds = newCheckbox("Show Line Bounds");
	showLineGuides = newCheckbox("Show Line Guides");

	basicExample.name = "Basic Paragraph of Text";
	basicExample.panel = uiControl(panel);
	basicExample.draw = draw;
	basicExample.mouse = NULL;
	basicExample.key = NULL;

	attrstr = uiNewAttributedString(text);
	params.String = attrstr;
	params.DefaultFont = &defaultFont;
	params.Align = uiDrawTextAlignLeft;

	return &basicExample;
}

// TODO on GTK+ an area by itself in a window doesn't get expand properties set properly?
