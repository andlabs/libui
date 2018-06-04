// 3 june 2018
#include "../ui.h"
#include "uipriv.h"

struct uiTableData {
	uiTableDataType type;
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

static uiTableData *newTableData(uiTableDataType type)
{
	uiTableData *d;

	d = uiprivNew(uiTableData);
	d->type = type;
	return d;
}

void uiFreeTableData(uiTableData *d)
{
	switch (d->type) {
	case uiTableDataTypeString:
		uiprivFree(d->u.str);
		break;
	}
	uiprivFree(d);
}

uiTableDataType uiTableDataGetType(const uiTableData *d)
{
	return d->type;
}

uiTableData *uiNewTableDataString(const char *str)
{
	uiTableData *d;

	d = newTableData(uiTableDataTypeString);
	d->u.str = (char *) uiprivAlloc((strlen(str) + 1) * sizeof (char), "char[] (uiTableData)");
	strcpy(d->u.str, str);
	return d;
}

const char *uiTableDataString(const uiTableData *d)
{
	return d->u.str;
}

uiTableData *uiNewTableDataImage(uiImage *img)
{
	uiTableData *d;

	d = newTableData(uiTableDataTypeImage);
	d->u.img = img;
	return d;
}

uiImage *uiTableDataImage(const uiTableData *d)
{
	return d->u.img;
}

uiTableData *uiNewTableDataInt(int i)
{
	uiTableData *d;

	d = newTableData(uiTableDataTypeInt);
	d->u.i = i;
	return d;
}

int uiTableDataInt(const uiTableData *d)
{
	return d->u.i;
}

uiTableData *uiNewTableDataColor(double r, double g, double b, double a)
{
	uiTableData *d;

	d = newTableData(uiTableDataTypeColor);
	d->u.color.r = r;
	d->u.color.g = g;
	d->u.color.b = b;
	d->u.color.a = a;
	return d;
}

void uiTableDataColor(const uiTableData *d, double *r, double *g, double *b, double *a)
{
	*r = d->u.color.r;
	*g = d->u.color.g;
	*b = d->u.color.b;
	*a = d->u.color.a;
}
