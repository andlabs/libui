
#include "uipriv_qt5.hpp"

#include "draw.hpp"

static QGradient uiDrawBrushToQGradient(const uiDrawBrush *b, QGradient gradient)
{
	for (size_t i=0; i<b->NumStops; ++i) {
		const auto &stop = b->Stops[i];
		const auto color = QColor::fromRgbF(stop.R, stop.G, stop.B, stop.A);
		gradient.setColorAt(stop.Pos, color);
	}
	return gradient;
}

static QBrush uiDrawBrushToQBrush(const uiDrawBrush *b)
{
	switch (b->Type)
	{
	case uiDrawBrushTypeSolid:
		return {QColor::fromRgbF(b->R, b->G, b->B, b->A), Qt::SolidPattern};
	case uiDrawBrushTypeLinearGradient:
		return uiDrawBrushToQGradient(b, QLinearGradient(b->X0, b->Y0, b->X1, b->Y1));
	case uiDrawBrushTypeRadialGradient:
		return uiDrawBrushToQGradient(b, QRadialGradient(b->X0, b->Y0, b->OuterRadius, b->X1, b->Y1, 0));
	case uiDrawBrushTypeImage:
		// not implemented in uiDrawBrush at time of writing
		qWarning("TODO: uiDrawBrushTypeImage");
		break;
	default:
		qWarning("Unknown uiDrawBrushType: %d", b->Type);
	}
	// something noticable
	return {Qt::magenta, Qt::DiagCrossPattern};
}

static QPen uiDrawStrokeParamsToQPen(const uiDrawStrokeParams *p, const QBrush &brush)
{
	QPen pen(brush, p->Thickness);

	switch (p->Cap)
	{
	case uiDrawLineCapFlat:
		pen.setCapStyle(Qt::FlatCap);
		break;
	case uiDrawLineCapRound:
		pen.setCapStyle(Qt::RoundCap);
		break;
	case uiDrawLineCapSquare:
		pen.setCapStyle(Qt::SquareCap);
		break;
	default:
		qWarning("Unknown uiDrawLineCap: %d", p->Cap);
	}

	switch (p->Join)
	{
	case uiDrawLineJoinMiter:
		pen.setJoinStyle(Qt::MiterJoin);
		break;
	case uiDrawLineJoinRound:
		pen.setJoinStyle(Qt::RoundJoin);
		break;
	case uiDrawLineJoinBevel:
		pen.setJoinStyle(Qt::BevelJoin);
		break;
	default:
		qWarning("Unknown uiDrawLineJoin: %d", p->Join);
	}

	pen.setMiterLimit(p->MiterLimit);

	if (p->NumDashes) {
		pen.setDashOffset(p->DashPhase/10.0);
		QVector<qreal> pattern;
		for (size_t i=0; i<p->NumDashes; ++i) {
			pattern.append(p->Dashes[i]/10.0);
		}
		pen.setDashPattern(pattern);
	}

	return pen;
}

void uiDrawStroke(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b, uiDrawStrokeParams *p)
{
	c->strokePath(*path, uiDrawStrokeParamsToQPen(p, uiDrawBrushToQBrush(b)));
}

void uiDrawFill(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b)
{
	c->fillPath(*path, uiDrawBrushToQBrush(b));
}

void uiDrawTransform(uiDrawContext *c, uiDrawMatrix *m)
{
	c->setTransform(m2t(m), true /* combine */);
}

void uiDrawClip(uiDrawContext *c, uiDrawPath *path)
{
	c->setClipPath(*path);
}

void uiDrawSave(uiDrawContext *c)
{
	c->save();
}

void uiDrawRestore(uiDrawContext *c)
{
	c->restore();
}
