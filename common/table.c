// 21 june 2016
#include "../ui.h"
#include "uipriv.h"

uiTableColumn *uiTableAppendTextColumn(uiTable *t, const char *name, int modelColumn)
{
	uiTableColumn *tc;

	tc = uiTableAppendColumn(t, name);
	uiTableColumnAppendTextPart(tc, modelColumn, 1);
	return tc;
}
