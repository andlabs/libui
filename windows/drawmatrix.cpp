// 7 september 2015
#include "uipriv_windows.hpp"
#include "draw.hpp"

// TODO convert to C++

void m2d(uiDrawMatrix *m, D2D1_MATRIX_3X2_F *d)
{
	d->_11 = m->M11;
	d->_12 = m->M12;
	d->_21 = m->M21;
	d->_22 = m->M22;
	d->_31 = m->M31;
	d->_32 = m->M32;
}

static void d2m(D2D1_MATRIX_3X2_F *d, uiDrawMatrix *m)
{
	m->M11 = d->_11;
	m->M12 = d->_12;
	m->M21 = d->_21;
	m->M22 = d->_22;
	m->M31 = d->_31;
	m->M32 = d->_32;
}

void uiDrawMatrixSetIdentity(uiDrawMatrix *m)
{
	setIdentity(m);
}

// frustratingly all of the operations on a matrix except rotation and skeweing are provided by the C++-only d2d1helper.h
// we'll have to recreate their functionalities here
// the implementations are all in the main matrix.c file
// TODO switch to these instead actually

void uiDrawMatrixTranslate(uiDrawMatrix *m, double x, double y)
{
	fallbackTranslate(m, x, y);
}

void uiDrawMatrixScale(uiDrawMatrix *m, double xCenter, double yCenter, double x, double y)
{
	fallbackScale(m, xCenter, yCenter, x, y);
}

void uiDrawMatrixRotate(uiDrawMatrix *m, double x, double y, double amount)
{
	D2D1_POINT_2F center;
	D2D1_MATRIX_3X2_F dm;
	uiDrawMatrix rm;

	amount *= 180 / M_PI;		// must be in degrees
	center.x = x;
	center.y = y;
	D2D1MakeRotateMatrix(amount, center, &dm);
	d2m(&dm, &rm);
	uiDrawMatrixMultiply(m, &rm);
}

void uiDrawMatrixSkew(uiDrawMatrix *m, double x, double y, double xamount, double yamount)
{
	D2D1_POINT_2F center;
	D2D1_MATRIX_3X2_F dm;
	uiDrawMatrix sm;

	xamount *= 180 / M_PI;		// must be in degrees
	yamount *= 180 / M_PI;		// must be in degrees
	center.x = x;
	center.y = y;
	D2D1MakeSkewMatrix(xamount, yamount, center, &dm);
	d2m(&dm, &sm);
	uiDrawMatrixMultiply(m, &sm);
}

void uiDrawMatrixMultiply(uiDrawMatrix *dest, uiDrawMatrix *src)
{
	fallbackMultiply(dest, src);
}

int uiDrawMatrixInvertible(uiDrawMatrix *m)
{
	D2D1_MATRIX_3X2_F d;

	m2d(m, &d);
	return D2D1IsMatrixInvertible(&d) != FALSE;
}

int uiDrawMatrixInvert(uiDrawMatrix *m)
{
	D2D1_MATRIX_3X2_F d;

	m2d(m, &d);
	if (D2D1InvertMatrix(&d) == FALSE)
		return 0;
	d2m(&d, m);
	return 1;
}

void uiDrawMatrixTransformPoint(uiDrawMatrix *m, double *x, double *y)
{
	fallbackTransformPoint(m, x, y);
}

void uiDrawMatrixTransformSize(uiDrawMatrix *m, double *x, double *y)
{
	fallbackTransformSize(m, x, y);
}
