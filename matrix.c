// 11 october 2015
#include "ui.h"
#include "uipriv.h"

void setIdentity(uiDrawMatrix *m)
{
	m->M11 = 1;
	m->M12 = 0;
	m->M21 = 0;
	m->M22 = 1;
	m->M31 = 0;
	m->M32 = 0;
}

// TODO skew

// see windows/draw.c for more information
// TODO we don't need to do this if we can bypass the multiplication somehow

void fallbackTranslate(uiDrawMatrix *m, double x, double y)
{
	uiDrawMatrix m2;

	setIdentity(&m2);
	m2.M31 = x;
	m2.M32 = y;
	fallbackMultiply(m, &m2);
}

// TODO the Direct2D version takes a center argument; investigate it
void fallbackScale(uiDrawMatrix *m, double x, double y)
{
	uiDrawMatrix m2;

	setIdentity(&m2);
	m2.M11 = x;
	m2.M22 = y;
	fallbackMultiply(m, &m2);
}

void fallbackMultiply(uiDrawMatrix *dest, uiDrawMatrix *src)
{
	uiDrawMatrix out;

	out.M11 = (dest->M11 * src->M11) +
		(dest->M12 * src->M21);
	out.M12 = (dest->M11 * src->M12) +
		(dest->M12 * src->M22);
	out.M21 = (dest->M21 * src->M11) +
		(dest->M22 * src->M21);
	out.M22 = (dest->M21 * src->M12) +
		(dest->M22 * src->M22);
	out.M31 = (dest->M31 * src->M11) +
		(dest->M32 * src->M21) +
		src->M31;
	out.M32 = (dest->M31 * src->M12) +
		(dest->M32 * src->M22) +
		src->M32;
	*dest = out;
}

void fallbackTransformPoint(uiDrawMatrix *m, double *x, double *y)
{
	double xout, yout;

	xout = (*x * m->M11) + (*y * m->M21) + m->M31;
	yout = (*x * m->M12) + (*y * m->M22) + m->M32;
	*x = xout;
	*y = yout;
}

// and this algorithm is according to cairo
void fallbackTransformSize(uiDrawMatrix *m, double *x, double *y)
{
	double xout, yout;

	xout = (*x * m->M11) + (*y * m->M21);
	yout = (*x * m->M12) + (*y * m->M22);
	*x = xout;
	*y = yout;
}
