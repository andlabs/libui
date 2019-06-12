// 11 june 2019

checkErrorCase(uiRegisterControlType(NULL, p->vtablePlaceholder, p->osVtablePlaceholder, p->implDataSizePlaceholder),
	"uiRegisterControlType(): invalid null pointer for uiControlOSVtable")
checkErrorCase(uiRegisterControlType(p->namePlaceholder, NULL, p->osVtablePlaceholder, p->implDataSizePlaceholder),
	"uiRegisterControlType(): invalid null pointer for uiControlVtable")
checkErrorCase(uiRegisterControlType(p->namePlaceholder, p->vtableBadSize, p->osVtablePlaceholder, p->implDataSizePlaceholder),
	"uiRegisterControlType(): wrong size 1 for uiControlVtable")
// TODO individual methods
checkErrorCase(uiRegisterControlType(p->namePlaceholder, p->vtablePlaceholder, NULL, p->implDataSizePlaceholder),
	"uiRegisterControlType(): invalid null pointer for uiControlOSVtable")
// OS vtable sizes are tested per-OS
