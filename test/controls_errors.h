// 11 june 2019

checkErrorCase(uiRegisterControlType(NULL, p->vtablePlaceholder, p->osVtablePlaceholder, p->implDataSizePlaceholder),
	"TODO")
checkErrorCase(uiRegisterControlType(p->namePlaceholder, NULL, p->osVtablePlaceholder, p->implDataSizePlaceholder),
	"TODO")
checkErrorCase(uiRegisterControlType(p->namePlaceholder, p->vtableBadSize, p->osVtablePlaceholder, p->implDataSizePlaceholder),
	"TODO")
// TODO individual methods
checkErrorCase(uiRegisterControlType(p->namePlaceholder, p->vtablePlaceholder, NULL, p->implDataSizePlaceholder),
	"TODO")
// OS vtable sizes are tested per-OS
