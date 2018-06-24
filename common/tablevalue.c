// 3 june 2018
#include "../ui.h"
#include "uipriv.h"
#include "table.h"

struct uiTableValue {
	uiTableValueType type;
	union {
		char *str;
		uiImage *img;
		int i;
		struct {
			double r;
			double g;
			double b;
			double a;
		} color;
	} u;
};

static uiTableValue *newTableValue(uiTableValueType type)
{
	uiTableValue *v;

	v = uiprivNew(uiTableValue);
	v->type = type;
	return v;
}

void uiFreeTableValue(uiTableValue *v)
{
	switch (v->type) {
	case uiTableValueTypeString:
		uiprivFree(v->u.str);
		break;
	}
	uiprivFree(v);
}

uiTableValueType uiTableValueGetType(const uiTableValue *v)
{
	return v->type;
}

uiTableValue *uiNewTableValueString(const char *str)
{
	uiTableValue *v;

	v = newTableValue(uiTableValueTypeString);
	v->u.str = (char *) uiprivAlloc((strlen(str) + 1) * sizeof (char), "char[] (uiTableValue)");
	strcpy(v->u.str, str);
	return v;
}

const char *uiTableValueString(const uiTableValue *v)
{
	return v->u.str;
}

uiTableValue *uiNewTableValueImage(uiImage *img)
{
	uiTableValue *v;

	v = newTableValue(uiTableValueTypeImage);
	v->u.img = img;
	return v;
}

uiImage *uiTableValueImage(const uiTableValue *v)
{
	return v->u.img;
}

uiTableValue *uiNewTableValueInt(int i)
{
	uiTableValue *v;

	v = newTableValue(uiTableValueTypeInt);
	v->u.i = i;
	return v;
}

int uiTableValueInt(const uiTableValue *v)
{
	return v->u.i;
}

uiTableValue *uiNewTableValueColor(double r, double g, double b, double a)
{
	uiTableValue *v;

	v = newTableValue(uiTableValueTypeColor);
	v->u.color.r = r;
	v->u.color.g = g;
	v->u.color.b = b;
	v->u.color.a = a;
	return v;
}

void uiTableValueColor(const uiTableValue *v, double *r, double *g, double *b, double *a)
{
	*r = v->u.color.r;
	*g = v->u.color.g;
	*b = v->u.color.b;
	*a = v->u.color.a;
}
