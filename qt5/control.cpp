
#include "uipriv_qt5.hpp"

#include <QWidget>
#include <QVariant>

#define uiQt5ControlSignature 0x51743578 // 'Qt5x';

Q_DECLARE_METATYPE(uiQt5Control *)

uiQt5Control *uiValidateQt5Control(uiControl *control)
{
	if (control == nullptr) {
		qCritical("Called with uiControl == nullptr");
		return nullptr;
	}
	if (control->Signature == uiQt5ControlSignature) {
		qCritical("Called with uiControl (%p) that does not contain correct uiQt5ControlSignature? "
				  "Corruption or incorrect object passed in!", (void*)control);
		return nullptr;
	}

	auto qt5Control = static_cast<uiQt5Control *>(control);;
	if (qt5Control->qobject == nullptr) {
		if ((qt5Control->flags & uiQt5Control::SuppressValidatationNag) == 0) {
			qCritical("Called with uiControl (%p) that doesn't have QObject (missing or already destroyed)", (void *)control);
		}
		return nullptr;
	}

	return qt5Control;
}

uiQt5Control *uiFindQt5ControlForQObject(const QObject *qobject)
{
	if (qobject) {
		return qobject->property("uiQt5Control").value<uiQt5Control *>();
	}
	return nullptr;
}

uiQt5Control *uiAllocQt5Control(uint32_t typesig, const char *typenamestr, QObject *qobject, uint32_t flags)
{
	auto control = (uiQt5Control *)uiAllocControl(sizeof(uiQt5Control), uiQt5ControlSignature, typesig, typenamestr);
	control->qobject = qobject;
	control->flags = flags;
	qobject->setProperty("uiQt5Control", QVariant::fromValue(control));
	QObject::connect(qobject, &QWidget::destroyed, qobject, [control]{
		control->qobject = nullptr;
		if ((control->flags & uiQt5Control::DeleteControlOnQObjectFree) == 1) {
			// supress warning about the QObject being already deleted
			control->flags |= uiQt5Control::SuppressValidatationNag;
			uiFreeControl(control);
		}
	});

	control->Destroy = [](uiControl *control) {
		if (auto qt5Control = uiValidateQt5Control(control)) {
			// consider synchronous destruction for simplicity? this is safer however depending on the caller state
			qt5Control->qobject->deleteLater();
			qt5Control->qobject->setProperty("uiQt5Control", {});

			// disconnect the destroyed signal and explicity set widget to null since the control
			// will be gone when the actual widget destruction happens
			QObject::disconnect(qt5Control->qobject, &QWidget::destroyed, qt5Control->qobject, nullptr);
			qt5Control->qobject = nullptr;

			uiFreeControl(control);
		}
	};
	control->Handle = [](uiControl *control) -> uintptr_t {
		if (auto qt5Control = uiValidateQt5Control(control)) {
			qWarning("VERIFY Handle usage");
			return (uintptr_t)qt5Control->qobject;
		}
		return 0;
	};
	control->Parent = [](uiControl *control) -> uiControl * {
		if (auto qt5Control = uiValidateQt5Control(control)) {
			qWarning("TODO Parent");
			Q_UNUSED(qt5Control);
			return nullptr;
		}
		return nullptr;
	};
	control->SetParent = [](uiControl *control, uiControl *parent) {
		if (auto qt5Control = uiValidateQt5Control(control)) {
			Q_UNUSED(parent);
			qWarning("TODO SetParent");
			Q_UNUSED(qt5Control);
		}
	};
	control->Toplevel = [](uiControl *control) -> int {
		if (auto qt5Control = uiValidateQt5Control(control)) {
			qWarning("TODO Toplevel");
			Q_UNUSED(qt5Control);
			return 0;
		}
		return 0;
	};
	control->Visible = [](uiControl *control) -> int {
		if (auto widget = uiValidateAndCastObjTo<QWidget>(control)) {
			return widget->isVisible();
		}
		return 0;
	};
	control->Show = [](uiControl *control) {
		if (auto widget = uiValidateAndCastObjTo<QWidget>(control)) {
			return widget->show();
		}
	};
	control->Hide = [](uiControl *control) {
		if (auto widget = uiValidateAndCastObjTo<QWidget>(control)) {
			return widget->hide();
		}
	};
	control->Enabled = [](uiControl *control) -> int {
		if (auto widget = uiValidateAndCastObjTo<QWidget>(control)) {
			return widget->isEnabled();
		}
		return 0;
	};
	control->Enable = [](uiControl *control) {
		if (auto widget = uiValidateAndCastObjTo<QWidget>(control)) {
			return widget->setEnabled(true);
		}
	};
	control->Disable = [](uiControl *control) {
		if (auto widget = uiValidateAndCastObjTo<QWidget>(control)) {
			return widget->setEnabled(false);
		}
	};

	return control;
}
