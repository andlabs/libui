
#include "uipriv_qt5.hpp"

#include "draw.hpp"

QTransform m2t(const uiDrawMatrix *m)
{
	return QTransform(m->M11, m->M12, m->M21, m->M22, m->M31, m->M32);
}

static void t2m(const QTransform &t, uiDrawMatrix *m)
{
	m->M11 = t.m11();
	m->M12 = t.m12();
	m->M21 = t.m21();
	m->M22 = t.m22();
	m->M31 = t.m31();
	m->M32 = t.m32();
}

void uiDrawMatrixTranslate(uiDrawMatrix *m, double x, double y)
{
	auto t = m2t(m);
	t.translate(x,y);
	t2m(t,m);
}

void uiDrawMatrixScale(uiDrawMatrix *m, double xCenter, double yCenter, double x, double y)
{
	auto t = m2t(m);
	t.translate(x,y);
	t.scale(x,y);
	t.translate(-x,-y);
	t2m(t,m);
}

void uiDrawMatrixRotate(uiDrawMatrix *m, double x, double y, double amount)
{
	auto t = m2t(m);
	t.translate(x,y);
	t.rotateRadians(amount);
	t.translate(-x,-y);
	t2m(t,m);
}

void uiDrawMatrixSkew(uiDrawMatrix *m, double x, double y, double xamount, double yamount)
{
	auto t = m2t(m);
	t.translate(x,y);
	t.shear(xamount, yamount);
	t.translate(-x,-y);
	t2m(t,m);
}

void uiDrawMatrixMultiply(uiDrawMatrix *dest, uiDrawMatrix *src)
{
	qWarning("TODO: %p, %p", (void *)dest, (void *)src);
}

int uiDrawMatrixInvertible(uiDrawMatrix *m)
{
	qWarning("TODO: %p", (void *)m);
	return 0;
}

int uiDrawMatrixInvert(uiDrawMatrix *m)
{
	qWarning("TODO: %p", (void *)m);
	return 0;
}

void uiDrawMatrixTransformPoint(uiDrawMatrix *m, double *x, double *y)
{
	qWarning("TODO: %p, %f %f", (void *)m, *x, *y);
}

void uiDrawMatrixTransformSize(uiDrawMatrix *m, double *x, double *y)
{
	qWarning("TODO: %p, %f %f", (void *)m, *x, *y);
}
