#pragma once

#include <QPainter>

typedef struct uiDrawContext uiDrawContext;

struct uiDrawContext : public QPainter
{
	uiDrawContext(QPaintDevice *device)
		: QPainter(device)
	{
	}
};

#include <QPainterPath>

struct uiDrawPath : QPainterPath
{
	uiDrawPath(uiDrawFillMode mode)
	{
		switch (mode)
		{
		case uiDrawFillModeWinding:
			setFillRule(Qt::WindingFill);
			break;
		case uiDrawFillModeAlternate:
			setFillRule(Qt::OddEvenFill);
			break;
		default:
			qWarning("Unknown uiDrawFillMode: %d", mode);
		}
	}
};

extern QTransform m2t(const uiDrawMatrix *m);

struct uiDrawTextFont : public QFont
{
	uiDrawTextFont(const QFont &font)
		: QFont(font) {}
};
