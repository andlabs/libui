
#include "uipriv_qt5.hpp"

#include "draw.hpp" // draw context

#include <QFontDatabase>
#include <QFontMetricsF>
#include <QTextLayout>

struct uiDrawFontFamilies : public QStringList
{
	uiDrawFontFamilies(const QStringList &stringList)
		: QStringList(stringList) {}
};

struct uiDrawTextLayout : public QTextLayout
{
	uiDrawTextLayout(const QString& text, const QFont &font, double width)
		: QTextLayout(text, font)
		, width_(width)
	{
//		qWarning("TODO: figure out wtf width is for: %f", width_);
	}
	double width_ = 0.0;
};

uiDrawFontFamilies *uiDrawListFontFamilies(void)
{
	return new uiDrawFontFamilies(QFontDatabase().families());
}

uintmax_t uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return ff->count();
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, uintmax_t n)
{
	return uiQt5StrdupQString(ff->value(n));
}

void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff)
{
	delete ff;
}

static int uiDrawTextWeightToFontWeight(uiDrawTextWeight weight)
{
	switch (weight)
	{
	case uiDrawTextWeightThin: return QFont::Thin;
	case uiDrawTextWeightUltraLight: return QFont::ExtraLight;
	case uiDrawTextWeightLight: return QFont::Light;
	case uiDrawTextWeightBook: return (QFont::Light+QFont::Normal)/2;
	case uiDrawTextWeightNormal: return QFont::Normal;
	case uiDrawTextWeightMedium: return QFont::Medium;
	case uiDrawTextWeightSemiBold: return QFont::DemiBold;
	case uiDrawTextWeightBold: return QFont::Bold;
	case uiDrawTextWeightUtraBold: return QFont::ExtraBold;
	case uiDrawTextWeightHeavy: return (QFont::ExtraBold+QFont::Black)/2;
	case uiDrawTextWeightUltraHeavy: return QFont::Black;
	default:
		qWarning("Unknown uiDrawTextWeight: %d", weight);
	}
	return QFont::Normal;
}

static int uiDrawTextItalicToFontStyle(uiDrawTextItalic italic)
{
	switch(italic)
	{
	case uiDrawTextItalicNormal: return QFont::StyleNormal;
	case uiDrawTextItalicOblique: return QFont::StyleOblique;
	case uiDrawTextItalicItalic: return QFont::StyleItalic;
	default:
		qWarning("Unknown uiDrawTextItalic: %d", italic);
	}
	return QFont::StyleNormal;
}

static int uiDrawTextStretchToFont(uiDrawTextStretch stretch)
{
	switch(stretch)
	{
	case uiDrawTextStretchUltraCondensed: return QFont::UltraCondensed;
	case uiDrawTextStretchExtraCondensed: return QFont::ExtraCondensed;
	case uiDrawTextStretchCondensed: return QFont::Condensed;
	case uiDrawTextStretchSemiCondensed: return QFont::SemiCondensed;
	case uiDrawTextStretchNormal: return QFont::Unstretched;
	case uiDrawTextStretchSemiExpanded: return QFont::SemiExpanded;
	case uiDrawTextStretchExpanded: return QFont::Expanded;
	case uiDrawTextStretchExtraExpanded: return QFont::ExtraExpanded;
	case uiDrawTextStretchUltraExpanded: return QFont::UltraExpanded;
	default:
		qWarning("Unknown uiDrawTextStretch: %d", stretch);
	}
	return QFont::Unstretched;
}

uiDrawTextFont *uiDrawLoadClosestFont(const uiDrawTextFontDescriptor *desc)
{
	QFont font(QString::fromUtf8(desc->Family));
	font.setPointSizeF(desc->Size);
	font.setWeight(uiDrawTextWeightToFontWeight(desc->Weight));
	font.setItalic(uiDrawTextItalicToFontStyle(desc->Italic));
	font.setStretch(uiDrawTextStretchToFont(desc->Stretch));

	return new uiDrawTextFont(font);
}

void uiDrawFreeTextFont(uiDrawTextFont *font)
{
	delete font;
}

uintptr_t uiDrawTextFontHandle(uiDrawTextFont *font)
{
	qWarning("TODO: %p", (void *)font);
	return 0;
}

void uiDrawTextFontDescribe(uiDrawTextFont *font, uiDrawTextFontDescriptor *desc)
{
	qWarning("TODO: %p %p", (void *)font, (void *)desc);
}

void uiDrawTextFontGetMetrics(uiDrawTextFont *font, uiDrawTextFontMetrics *metrics)
{
	if (font) {
		QFontMetricsF fontMetrics(*font);
		metrics->Ascent = fontMetrics.ascent();
		metrics->Descent = fontMetrics.descent();
		metrics->Leading = fontMetrics.leading();
		metrics->UnderlinePos = fontMetrics.underlinePos();
		metrics->UnderlineThickness = 0; // TODO?
	}
}

uiDrawTextLayout *uiDrawNewTextLayout(const char *text, uiDrawTextFont *defaultFont, double width)
{
	return new uiDrawTextLayout(QString::fromUtf8(text), *defaultFont, width);
}

void uiDrawFreeTextLayout(uiDrawTextLayout *layout)
{
	delete layout;
}

void uiDrawTextLayoutSetWidth(uiDrawTextLayout *layout, double width)
{
	layout->width_ = width;
}

static void runLayouting(uiDrawTextLayout *layout)
{
	layout->setCacheEnabled(true);
	layout->beginLayout();
	forever {
		QTextLine line = layout->createLine();
		if (!line.isValid()) {
			break;
		}
		// TODO: shift by line height?
	}
	layout->endLayout();
}

void uiDrawTextLayoutExtents(uiDrawTextLayout *layout, double *width, double *height)
{
	runLayouting(layout);
	auto boundingRect = layout->boundingRect();
	*width = boundingRect.width();
	*height = boundingRect.height();
}

void uiDrawText(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout)
{
	runLayouting(layout);
	layout->draw(c, {x,y});
}

void uiDrawTextLayoutSetColor(uiDrawTextLayout *layout, intmax_t startChar, intmax_t endChar, double r, double g, double b, double a)
{
	// ineffective..? but works
	auto additionalFormats = layout->additionalFormats();
	QTextCharFormat format;
	format.setForeground(QColor::fromRgbF(r,g,b,a));
	additionalFormats.append(QTextLayout::FormatRange{(int)startChar,(int)(1+endChar-startChar),format});
	layout->setAdditionalFormats(additionalFormats);
}
