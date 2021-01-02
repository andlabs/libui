
#include "uipriv_qt5.hpp"

#include <QGroupBox>

#include <QVBoxLayout>

struct uiGroup : public uiQt5Control {};

char *uiGroupTitle(uiGroup *g)
{
	if (auto groupBox = uiValidateAndCastObjTo<QGroupBox>(g)) {
		return uiQt5StrdupQString(groupBox->title());
	}
	return nullptr;
}

void uiGroupSetTitle(uiGroup *g, const char *text)
{
	if (auto groupBox = uiValidateAndCastObjTo<QGroupBox>(g)) {
		groupBox->setTitle(QString::fromUtf8(text));
	}
}

void uiGroupSetChild(uiGroup *g, uiControl *child)
{
	if (auto groupBox = uiValidateAndCastObjTo<QGroupBox>(g)) {
		auto obj = uiValidateAndCastObjTo<QObject>(child);
		if (groupBox->layout()) {
			groupBox->layout()->deleteLater();
		}
		if (auto layout = qobject_cast<QLayout*>(obj)) {
			groupBox->setLayout(layout);
		} else if (auto widget = qobject_cast<QWidget*>(obj)) {
			auto layout = new QVBoxLayout;
			layout->setMargin(0); // ?
			layout->addWidget(widget);
			groupBox->setLayout(layout);
		} else {
			qWarning("object is neither layout nor widget");
		}
	}
}

int uiGroupMargined(uiGroup *g)
{
	qWarning("TODO: %p", (void*)g);
	return 0;
}

void uiGroupSetMargined(uiGroup *g, int margined)
{
	qWarning("TODO: %p, %d", (void*)g, margined);
}

uiGroup *uiNewGroup(const char *text)
{
	auto groupBox = new QGroupBox(QString::fromUtf8(text));

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiGroup,groupBox,uiQt5Control::DeleteControlOnQObjectFree);
}
