// 29 may 2015
#include "ui.h"

static uintmax_t type_uiMenu = 0;
static uintmax_t type_uiMenuItem = 0;

uintmax_t uiTypeMenu(void)
{
	if (type_uiMenu == 0)
		type_uiMenu = uiRegisterType("uiMenu", 0, 0);
	return type_uiMenu;
}

uintmax_t uiTypeMenuItem(void)
{
	if (type_uiMenuItem == 0)
		type_uiMenuItem = uiRegisterType("uiMenuItem", 0, 0);
	return type_uiMenuItem;
}
