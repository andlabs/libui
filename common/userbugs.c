// 22 may 2016
#include "../ui.h"
#include "uipriv.h"

void uiUserBugCannotSetParentOnToplevel(const char *type)
{
	uiprivUserBug("You cannot make a %s a child of another uiControl,", type);
}
