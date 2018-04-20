// 19 february 2018
#include "../ui.h"
#include "uipriv.h"
#include "attrstr.h"

struct uiAttribute {
	int ownedByUser;
	size_t refcount;
	uiAttributeType type;
	union {
		char *family;
		double size;
		uiTextWeight weight;
		uiTextItalic italic;
		uiTextStretch stretch;
		struct {
			double r;
			double g;
			double b;
			double a;
			// put this here so we can reuse this structure
			uiUnderlineColor underlineColor;
		} color;
		uiUnderline underline;
		uiOpenTypeFeatures *features;
	} u;
};

static uiAttribute *newAttribute(uiAttributeType type)
{
	uiAttribute *a;

	a = uiprivNew(uiAttribute);
	a->ownedByUser = 1;
	a->refcount = 0;
	a->type = type;
	return a;
}

// returns a to allow expressions like b = uiprivAttributeRetain(a)
// TODO would this allow us to copy attributes between strings in a foreach func, and if so, should that be allowed?
uiAttribute *uiprivAttributeRetain(uiAttribute *a)
{
	a->ownedByUser = 0;
	a->refcount++;
	return a;
}

static void destroy(uiAttribute *a)
{
	switch (a->type) {
	case uiAttributeTypeFamily:
		uiprivFree(a->u.family);
		break;
	case uiAttributeTypeFeatures:
		uiFreeOpenTypeFeatures(a->u.features);
		break;
	}
	uiprivFree(a);
}

void uiprivAttributeRelease(uiAttribute *a)
{
	if (a->ownedByUser)
		/* TODO implementation bug: we can't release an attribute we don't own */;
	a->refcount--;
	if (a->refcount == 0)
		destroy(a);
}

void uiFreeAttribute(uiAttribute *a)
{
	if (!a->ownedByUser)
		/* TODO user bug: you can't free an attribute you don't own */;
	destroy(a);
}

uiAttributeType uiAttributeGetType(const uiAttribute *a)
{
	return a->type;
}

uiAttribute *uiNewFamilyAttribute(const char *family)
{
	uiAttribute *a;

	a = newAttribute(uiAttributeTypeFamily);
	a->u.family = (char *) uiprivAlloc((strlen(family) + 1) * sizeof (char), "char[] (uiAttribute)");
	strcpy(a->u.family, family);
	return a;
}

const char *uiAttributeFamily(const uiAttribute *a)
{
	return a->u.family;
}

uiAttribute *uiNewSizeAttribute(double size)
{
	uiAttribute *a;

	a = newAttribute(uiAttributeTypeSize);
	a->u.size = size;
	return a;
}

double uiAttributeSize(const uiAttribute *a)
{
	return a->u.size;
}

uiAttribute *uiNewWeightAttribute(uiTextWeight weight)
{
	uiAttribute *a;

	a = newAttribute(uiAttributeTypeWeight);
	a->u.weight = weight;
	return a;
}

uiTextWeight uiAttributeWeight(const uiAttribute *a)
{
	return a->u.weight;
}

uiAttribute *uiNewItalicAttribute(uiTextItalic italic)
{
	uiAttribute *a;

	a = newAttribute(uiAttributeTypeItalic);
	a->u.italic = italic;
	return a;
}

uiTextItalic uiAttributeItalic(const uiAttribute *a)
{
	return a->u.italic;
}

uiAttribute *uiNewStretchAttribute(uiTextStretch stretch)
{
	uiAttribute *a;

	a = newAttribute(uiAttributeTypeStretch);
	a->u.stretch = stretch;
	return a;
}

uiTextStretch uiAttributeStretch(const uiAttribute *a)
{
	return a->u.stretch;
}

uiAttribute *uiNewColorAttribute(double r, double g, double b, double a)
{
	uiAttribute *at;

	at = newAttribute(uiAttributeTypeColor);
	at->u.color.r = r;
	at->u.color.g = g;
	at->u.color.b = b;
	at->u.color.a = a;
	return at;
}

void uiAttributeColor(const uiAttribute *a, double *r, double *g, double *b, double *alpha)
{
	*r = a->u.color.r;
	*g = a->u.color.g;
	*b = a->u.color.b;
	*alpha = a->u.color.a;
}

uiAttribute *uiNewBackgroundAttribute(double r, double g, double b, double a)
{
	uiAttribute *at;

	at = newAttribute(uiAttributeTypeBackground);
	at->u.color.r = r;
	at->u.color.g = g;
	at->u.color.b = b;
	at->u.color.a = a;
	return at;
}

uiAttribute *uiNewUnderlineAttribute(uiUnderline u)
{
	uiAttribute *a;

	a = newAttribute(uiAttributeTypeUnderline);
	a->u.underline = u;
	return a;
}

uiUnderline uiAttributeUnderline(const uiAttribute *a)
{
	return a->u.underline;
}

uiAttribute *uiNewUnderlineColorAttribute(uiUnderlineColor u, double r, double g, double b, double a)
{
	uiAttribute *at;

	at = uiNewColorAttribute(r, g, b, a);
	at->type = uiAttributeTypeUnderlineColor;
	at->u.color.underlineColor = u;
	return at;
}

void uiAttributeUnderlineColor(const uiAttribute *a, uiUnderlineColor *u, double *r, double *g, double *b, double *alpha)
{
	*u = a->u.color.underlineColor;
	uiAttributeColor(a, r, g, b, alpha);
}

uiAttribute *uiNewFeaturesAttribute(const uiOpenTypeFeatures *otf)
{
	uiAttribute *a;

	a = newAttribute(uiAttributeTypeFeatures);
	a->u.features = uiOpenTypeFeaturesClone(otf);
	return a;
}

const uiOpenTypeFeatures *uiAttributeFeatures(const uiAttribute *a)
{
	return a->u.features;
}

int uiprivAttributeEqual(const uiAttribute *a, const uiAttribute *b)
{
	if (a == b)
		return 1;
	if (a->type != b->type)
		return 0;
	switch (a->type) {
	case uiAttributeTypeFamily:
		return uiprivStricmp(a->u.family, b->u.family);
	case uiAttributeTypeSize:
		// TODO is the use of == correct?
		return a->u.size == b->u.size;
	case uiAttributeTypeWeight:
		return a->u.weight == b->u.weight;
	case uiAttributeTypeItalic:
		return a->u.italic == b->u.italic;
	case uiAttributeTypeStretch:
		return a->u.stretch == b->u.stretch;
	case uiAttributeTypeUnderline:
		return a->u.underline == b->u.underline;
	case uiAttributeTypeUnderlineColor:
		if (a->u.color.underlineColor != b->u.color.underlineColor)
			return 0;
		// fall through
	case uiAttributeTypeColor:
	case uiAttributeTypeBackground:
		// TODO is the use of == correct?
		return (a->u.color.r == b->u.color.r) &&
			(a->u.color.g == b->u.color.g) &&
			(a->u.color.b == b->u.color.b) &&
			(a->u.color.a == b->u.color.a);
	case uiAttributeTypeFeatures:
		return uiprivOpenTypeFeaturesEqual(a->u.features, b->u.features);
	}
	// TODO should not be reached
	return 0;
}
