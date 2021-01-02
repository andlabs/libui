
#include "uipriv_qt5.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QLayoutItem>
#include <QWidget>

struct uiBox : public uiQt5Control {};

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	if (auto layoutBox = uiValidateAndCastObjTo<QBoxLayout>(b)) {
		auto obj = uiValidateAndCastObjTo<QObject>(c);

		if (auto layout = qobject_cast<QLayout*>(obj)) {
			layoutBox->addLayout(layout, stretchy);
		} else if (auto widget = qobject_cast<QWidget*>(obj)) {
			layoutBox->addWidget(widget);
		} else {
			qWarning("object is neither layout nor widget");
		}
	}
}

void uiBoxDelete(uiBox *b, uintmax_t index)
{
	if (auto layoutBox = uiValidateAndCastObjTo<QBoxLayout>(b)) {
		if (index < (uint)layoutBox->count()) {
			delete layoutBox->takeAt(index);
		}
	}
}

int uiBoxPadded(uiBox *b)
{
	if (auto layoutBox = uiValidateAndCastObjTo<QBoxLayout>(b)) {
		layoutBox->spacing();
	}
	return 0;
}

void uiBoxSetPadded(uiBox *b, int padded)
{
	if (auto layoutBox = uiValidateAndCastObjTo<QBoxLayout>(b)) {
		layoutBox->setSpacing(padded);
	}
}

uiBox *uiNewHorizontalBox(void)
{
	return uiAllocQt5ControlType(uiBox, new QHBoxLayout, uiQt5Control::DeleteControlOnQObjectFree);
}

uiBox *uiNewVerticalBox(void)
{
	auto layout = new QVBoxLayout;
	layout->setAlignment(Qt::AlignTop);

	return uiAllocQt5ControlType(uiBox, layout, uiQt5Control::DeleteControlOnQObjectFree);
}
