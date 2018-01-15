// 22 december 2015
#include "uipriv_windows.hpp"
#include "draw.hpp"
// TODO really migrate

// notes:
// only available in windows 8 and newer:
// - character spacing
// - kerning control
// - justficiation (how could I possibly be making this up?!)
// - vertical text (SERIOUSLY?! WHAT THE ACTUAL FUCK, MICROSOFT?!?!?!? DID YOU NOT THINK ABOUT THIS THE FIRST TIME, TRYING TO IMPROVE THE INTERNATIONALIZATION OF WINDOWS 7?!?!?! bonus: some parts of MSDN even say 8.1 and up only!)

struct uiDrawFontFamilies {
	fontCollection *fc;
};

uiDrawFontFamilies *uiDrawListFontFamilies(void)
{
	struct uiDrawFontFamilies *ff;

	ff = uiNew(struct uiDrawFontFamilies);
	ff->fc = loadFontCollection();
	return ff;
}

int uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return ff->fc->fonts->GetFontFamilyCount();
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, int n)
{
	IDWriteFontFamily *family;
	WCHAR *wname;
	char *name;
	HRESULT hr;

	hr = ff->fc->fonts->GetFontFamily(n, &family);
	if (hr != S_OK)
		logHRESULT(L"error getting font out of collection", hr);
	wname = fontCollectionFamilyName(ff->fc, family);
	name = toUTF8(wname);
	uiFree(wname);
	family->Release();
	return name;
}

void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff)
{
	fontCollectionFree(ff->fc);
	uiFree(ff);
}

struct uiDrawTextFont {
	IDWriteFont *f;
	WCHAR *family;		// save for convenience in uiDrawNewTextLayout()
	double size;
};

uiDrawTextFont *mkTextFont(IDWriteFont *df, BOOL addRef, WCHAR *family, BOOL copyFamily, double size)
{
	uiDrawTextFont *font;
	WCHAR *copy;
	HRESULT hr;

	font = uiNew(uiDrawTextFont);
	font->f = df;
	if (addRef)
		font->f->AddRef();
	if (copyFamily) {
		copy = (WCHAR *) uiAlloc((wcslen(family) + 1) * sizeof (WCHAR), "WCHAR[]");
		wcscpy(copy, family);
		font->family = copy;
	} else
		font->family = family;
	font->size = size;
	return font;
}

// TODO MinGW-w64 is missing this one
#define DWRITE_FONT_WEIGHT_SEMI_LIGHT (DWRITE_FONT_WEIGHT(350))

uiDrawTextFont *uiDrawLoadClosestFont(const uiDrawTextFontDescriptor *desc)
{
	uiDrawTextFont *font;
	IDWriteFontCollection *collection;
	UINT32 index;
	BOOL exists;
	struct dwriteAttr attr;
	IDWriteFontFamily *family;
	WCHAR *wfamily;
	IDWriteFont *match;
	HRESULT hr;

	// always get the latest available font information
	hr = dwfactory->GetSystemFontCollection(&collection, TRUE);
	if (hr != S_OK)
		logHRESULT(L"error getting system font collection", hr);

	wfamily = toUTF16(desc->Family);
	hr = collection->FindFamilyName(wfamily, &index, &exists);
	if (hr != S_OK)
		logHRESULT(L"error finding font family", hr);
	if (!exists)
		implbug("LONGTERM family not found in uiDrawLoadClosestFont()", hr);
	hr = collection->GetFontFamily(index, &family);
	if (hr != S_OK)
		logHRESULT(L"error loading font family", hr);

	attr.weight = desc->Weight;
	attr.italic = desc->Italic;
	attr.stretch = desc->Stretch;
	attrToDWriteAttr(&attr);
	hr = family->GetFirstMatchingFont(
		attr.dweight,
		attr.dstretch,
		attr.ditalic,
		&match);
	if (hr != S_OK)
		logHRESULT(L"error loading font", hr);

	font = mkTextFont(match,
		FALSE,				// we own the initial reference; no need to add another one
		wfamily, FALSE,		// will be freed with font
		desc->Size);

	family->Release();
	collection->Release();

	return font;
}

void uiDrawFreeTextFont(uiDrawTextFont *font)
{
	font->f->Release();
	uiFree(font->family);
	uiFree(font);
}

uintptr_t uiDrawTextFontHandle(uiDrawTextFont *font)
{
	return (uintptr_t) (font->f);
}

void uiDrawTextFontDescribe(uiDrawTextFont *font, uiDrawTextFontDescriptor *desc)
{
	// TODO

	desc->Size = font->size;

	// TODO
}

// text sizes are 1/72 of an inch
// points in Direct2D are 1/96 of an inch (https://msdn.microsoft.com/en-us/library/windows/desktop/ff684173%28v=vs.85%29.aspx, https://msdn.microsoft.com/en-us/library/windows/desktop/hh447022%28v=vs.85%29.aspx)
// As for the actual conversion from design units, see:
// - http://cboard.cprogramming.com/windows-programming/136733-directwrite-font-height-issues.html
// - https://sourceforge.net/p/vstgui/mailman/message/32483143/
// - http://xboxforums.create.msdn.com/forums/t/109445.aspx
// - https://msdn.microsoft.com/en-us/library/dd183564%28v=vs.85%29.aspx
// - http://www.fontbureau.com/blog/the-em/
// TODO make points here about how DIPs in DirectWrite == DIPs in Direct2D; if not, figure out what they really are? for the width and layout functions later
static double scaleUnits(double what, double designUnitsPerEm, double size)
{
	return (what / designUnitsPerEm) * (size * (96.0 / 72.0));
}

void uiDrawTextFontGetMetrics(uiDrawTextFont *font, uiDrawTextFontMetrics *metrics)
{
	DWRITE_FONT_METRICS dm;

	font->f->GetMetrics(&dm);
	metrics->Ascent = scaleUnits(dm.ascent, dm.designUnitsPerEm, font->size);
	metrics->Descent = scaleUnits(dm.descent, dm.designUnitsPerEm, font->size);
	// TODO what happens if dm.xxx is negative?
	// TODO remember what this was for
	metrics->Leading = scaleUnits(dm.lineGap, dm.designUnitsPerEm, font->size);
	metrics->UnderlinePos = scaleUnits(dm.underlinePosition, dm.designUnitsPerEm, font->size);
	metrics->UnderlineThickness = scaleUnits(dm.underlineThickness, dm.designUnitsPerEm, font->size);
}

// some attributes, such as foreground color, can't be applied until after we establish a Direct2D context :/ so we have to prepare all attributes in advance
// also since there's no way to clear the attributes from a layout en masse (apart from overwriting them all), we'll play it safe by creating a new layout each time
enum layoutAttrType {
	layoutAttrColor,
};

struct layoutAttr {
	enum layoutAttrType type;
	int start;
	int end;
	double components[4];
};

struct uiDrawTextLayout {
	WCHAR *text;
	size_t textlen;
	size_t *graphemes;
	double width;
	IDWriteTextFormat *format;
	std::vector<struct layoutAttr> *attrs;
};

uiDrawTextLayout *uiDrawNewTextLayout(const char *text, uiDrawTextFont *defaultFont, double width)
{
	uiDrawTextLayout *layout;
	HRESULT hr;

	layout = uiNew(uiDrawTextLayout);

	hr = dwfactory->CreateTextFormat(defaultFont->family,
		NULL,
		defaultFont->f->GetWeight(),
		defaultFont->f->GetStyle(),
		defaultFont->f->GetStretch(),

	if (hr != S_OK)
		logHRESULT(L"error creating IDWriteTextFormat", hr);

	layout->text = toUTF16(text);
	layout->textlen = wcslen(layout->text);
	layout->graphemes = graphemes(layout->text);

	uiDrawTextLayoutSetWidth(layout, width);

	layout->attrs = new std::vector<struct layoutAttr>;

	return layout;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *layout)
{
	delete layout->attrs;
	layout->format->Release();
	uiFree(layout->graphemes);
	uiFree(layout->text);
	uiFree(layout);
}


IDWriteTextLayout *prepareLayout(uiDrawTextLayout *layout, ID2D1RenderTarget *rt)
{
	IDWriteTextLayout *dl;
	DWRITE_TEXT_RANGE range;
	IUnknown *unkBrush;
	HRESULT hr;

	for (const struct layoutAttr &attr : *(layout->attrs)) {
		range.startPosition = layout->graphemes[attr.start];
		range.length = layout->graphemes[attr.end] - layout->graphemes[attr.start];
		switch (attr.type) {
		case layoutAttrColor:
			if (rt == NULL)		// determining extents, not drawing
				break;
			unkBrush = mkSolidBrush(rt,
				attr.components[0],
				attr.components[1],
				attr.components[2],
				attr.components[3]);
			hr = dl->SetDrawingEffect(unkBrush, range);
			unkBrush->Release();		// associated with dl
			break;
		default:
			hr = E_FAIL;
			logHRESULT(L"invalid text attribute type", hr);
		}
		if (hr != S_OK)
			logHRESULT(L"error adding attribute to text layout", hr);
	}



	return dl;
}


void uiDrawText(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout)
{
	IDWriteTextLayout *dl;
	D2D1_POINT_2F pt;
	ID2D1Brush *black;
	HRESULT hr;

	// TODO document that fully opaque black is the default text color; figure out whether this is upheld in various scenarios on other platforms
	black = mkSolidBrush(c->rt, 0.0, 0.0, 0.0, 1.0);

	dl = prepareLayout(layout, c->rt);
	pt.x = x;
	pt.y = y;
	// TODO D2D1_DRAW_TEXT_OPTIONS_NO_SNAP?
	// TODO D2D1_DRAW_TEXT_OPTIONS_CLIP?
	// TODO when setting 8.1 as minimum, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT?
	c->rt->DrawTextLayout(pt, dl, black, D2D1_DRAW_TEXT_OPTIONS_NONE);
	dl->Release();

	black->Release();
}

void uiDrawTextLayoutSetColor(uiDrawTextLayout *layout, int startChar, int endChar, double r, double g, double b, double a)
{
	struct layoutAttr attr;

	attr.type = layoutAttrColor;
	attr.start = startChar;
	attr.end = endChar;
	attr.components[0] = r;
	attr.components[1] = g;
	attr.components[2] = b;
	attr.components[3] = a;
	layout->attrs->push_back(attr);
}
